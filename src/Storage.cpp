#include "Logging.h"
#include "RateLimiter.h"
#include "Storage.h"

#include <chrono>
#include <rocksdb/table.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/cache.h>
#include <rocksdb/slice_transform.h>
#include <folly/ThreadName.h>

using namespace rocksdb;
using namespace std::chrono;

DECLARE_LOG_OBJECT();

constexpr unsigned long long int operator ""_KB(unsigned long long int kilobytes) {
    return kilobytes * 1024;
}

constexpr unsigned long long int operator ""_MB(unsigned long long int megabytes) {
    return megabytes * 1024 * 1024;
}

constexpr unsigned long long int operator ""_GB(unsigned long long int gigabytes) {
    return gigabytes * 1024 * 1024 * 1024;
}

Storage::Storage(const BookieConfig& conf, MetricsManager& metricsManager) :
        db_(nullptr),
        writeOptions_(),
        journalQueue_(10000),
        fsyncWal_(conf.fsyncWal()),
        journalThread_(std::bind(&Storage::runJournal, this)),
        rocksDbPutLatency_(metricsManager.createMetric("rocksDbPut")),
        addEntryEnqueueLatency_(metricsManager.createMetric("addEntryEnqueueLatency")),
        walSyncLatency_(metricsManager.createMetric("walSync")),
        walQueueLatency_(metricsManager.createMetric("walQueueLatency")) {
    Options options;
    options.create_if_missing = true;
    options.write_buffer_size = 1_GB;
    options.max_write_buffer_number = 4;
    options.max_background_compactions = 16;
    options.max_background_flushes = 4;
    options.IncreaseParallelism(std::thread::hardware_concurrency());
    options.max_open_files = -1;
    options.max_file_opening_threads = 16;
    options.target_file_size_base = 1_GB;
    options.max_bytes_for_level_base = 10_GB;
    options.delete_obsolete_files_period_micros = duration_cast<microseconds>(hours(1)).count();
    options.compaction_readahead_size = 8_MB;
    options.allow_concurrent_memtable_write = true;

    // Keys are always 16 bytes (ledgerId, entryId)
    options.prefix_extractor.reset(NewFixedPrefixTransform(8));

    options.log_file_time_to_roll = duration_cast<seconds>(hours(24)).count();
    options.keep_log_file_num = 30;
    options.stats_dump_period_sec = 60;

    options.wal_dir = conf.walDirectory();

    BlockBasedTableOptions table_options;
    table_options.block_size = 256_KB;
    table_options.format_version = 2;
    table_options.checksum = kxxHash;
    table_options.block_cache = NewLRUCache(8_GB, 8);
    table_options.cache_index_and_filter_blocks = true;
    table_options.filter_policy.reset(NewBloomFilterPolicy(10, false));
    options.table_factory.reset(NewBlockBasedTableFactory(table_options));

    LOG_INFO("Opening database at " << conf.dataDirectory());

    Status res = DB::Open(options, conf.dataDirectory(), &db_);
    if (!res.ok()) {
        LOG_FATAL("Failed to open database: " << res.code());
        std::exit(1);
    }

    LOG_INFO("Database opened successfully");
}

Storage::~Storage() {
    // Write a null promise to make the journal thread to exit
    JournalEntry entry { { }, { }, nullptr, walQueueLatency_->startTimer() };
    journalQueue_.blockingWrite(std::move(entry));
    journalThread_.join();
    delete db_;
}

Future<Unit> Storage::put(int64_t ledgerId, int64_t entryId, IOBufPtr data) {
    PromisePtr promise = make_unique<Promise<Unit>>();
    Future<Unit> future = promise->getFuture();

    union Key {
        struct {
            int64_t ledgerId;
            int64_t entryId;
        };
        char data[0];
    };

    Key key;
    key.ledgerId = Endian::big(ledgerId);
    key.entryId = Endian::big(entryId);

    JournalEntry entry { Slice(key.data, sizeof(key)), std::move(data), std::move(promise),
            walQueueLatency_->startTimer() };

    Slice valueSlice((const char*) data->data(), data->length());

    Timer addEntryEnqueueTimer = addEntryEnqueueLatency_->startTimer();
    journalQueue_.blockingWrite(std::move(entry));
    addEntryEnqueueTimer.completed();

    return future;
}

void Storage::runJournal() {
    setThreadName("bookie-journal");

    std::vector<PromisePtr> entriesToSync;
    Unit unit;
    Metric* journalSyncLatency = walSyncLatency_.get();
    WriteOptions syncOptions;
    syncOptions.sync = fsyncWal_;
    WriteBatch writeBatch;

    JournalEntry entry;
    bool blockForNextEntry = false;

    while (true) {
        // Collect all items from queue
        int toSyncCount = 0;

        while (true) {
            if (blockForNextEntry) {
                journalQueue_.blockingRead(entry);
                blockForNextEntry = false;
            } else {
                if (!journalQueue_.read(entry)) {
                    blockForNextEntry = true;
                    if (toSyncCount == 0) {
                        // Block until new entry is available
                        continue;
                    } else {
                        // Queue is drained, write entries to db
                        break;
                    }
                }
            }

            if (entry.promise.get() == nullptr) {
                // Journal is exiting
                return;
            }

            entry.walTimeSpentInQueue.completed();
            entriesToSync.emplace_back(std::move(entry.promise));
            writeBatch.Put(entry.key, Slice((const char*) entry.data->data(), entry.data->length()));

            if (toSyncCount++ == 1000) {
                break;
            }
        }

        if (entriesToSync.empty()) {
            continue;
        }

        Timer syncLatencyTimer = journalSyncLatency->startTimer();
        db_->Write(syncOptions, &writeBatch);
        syncLatencyTimer.completed();

        for (auto& pr : entriesToSync) {
            pr->setValue(unit);
        }

        entriesToSync.clear();
        writeBatch.Clear();
    }
}
