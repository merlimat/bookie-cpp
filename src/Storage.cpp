
#include "Logging.h"
#include "RateLimiter.h"
#include "Storage.h"

#include <rocksdb/table.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/cache.h>
#include <rocksdb/slice_transform.h>
#include <folly/ThreadName.h>

using namespace rocksdb;

DECLARE_LOG_OBJECT();

Storage::Storage(const BookieConfig& conf) :
        db_(nullptr),
        writeOptions_(),
        journalQueue_(10000),
        journalThread_(std::bind(&Storage::runJournal, this)) {
    Options options;
    options.create_if_missing = true;
    options.write_buffer_size = 1024 * 1024 * 1024L;
    options.max_write_buffer_number = 4;
    options.max_background_compactions = 16;
    options.max_background_flushes = 4;
    options.IncreaseParallelism(std::thread::hardware_concurrency());
    options.max_open_files = -1;
    options.max_file_opening_threads = 16;
    options.target_file_size_base = 64 * 1024 * 1024L;
    options.max_bytes_for_level_base = 256 * 1024 * 1024L;
    options.delete_obsolete_files_period_micros = 3600 * 1000 * 1000L;
    options.level_compaction_dynamic_level_bytes = true;

    // Keys are always 16 bytes (ledgerId, entryId)
    options.prefix_extractor.reset(NewFixedPrefixTransform(8));

    options.log_file_time_to_roll = 3600 * 24;
    options.keep_log_file_num = 30;
    options.stats_dump_period_sec = 60;

    options.wal_dir = conf.walDirectory();

    BlockBasedTableOptions table_options;
    table_options.block_size = 64 * 1024;
    table_options.format_version = 2;
    table_options.checksum = kxxHash;
    table_options.block_cache = NewLRUCache(8L * 1024 * 1024 * 1024, 8);
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
    journalQueue_.blockingWrite(nullptr);
    journalThread_.join();
    delete db_;
}

Future<Unit> Storage::put(const Slice& key, const Slice& value) {
    Status res = db_->Put(writeOptions_, nullptr, key, value);
    if (res.ok()) {
        PromisePtr promise = make_unique<Promise<Unit>>();
        Future<Unit> future = promise->getFuture();
        journalQueue_.write(std::move(promise));

        return future;
    } else {
        LOG_ERROR("Failed to write to db. res: " << res.code());
        return makeFuture<Unit>(std::runtime_error("Failed to write to db"));
    }
}

void Storage::runJournal() {
    setThreadName("journal");

    std::vector<PromisePtr> entriesToSync;

    ::RateLimiter limiter(5000);
    Unit unit;

    while (true) {
        limiter.aquire();

        // Collect all items from queue
        PromisePtr promise;
        while (journalQueue_.read(promise)) {
            if (promise.get() == nullptr) {
                // Journal is exiting
                return;
            }
            entriesToSync.emplace_back(std::move(promise));
        }

        if (entriesToSync.empty()) {
            continue;
        }

        db_->SyncWAL();

        for (auto& pr : entriesToSync) {
            pr->setValue(unit);
        }

        entriesToSync.clear();
    }
}
