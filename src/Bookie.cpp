#include "Bookie.h"
#include "Logging.h"

#include <folly/io/async/EventBaseManager.h>

DECLARE_LOG_OBJECT();

Bookie::Bookie(const BookieConfig& conf) :
        conf_(conf),
        zk_(conf.zkServers(), std::chrono::milliseconds(conf.zkSessionTimeout())),
        bookieRegistration_(&zk_, conf),
        storage_(conf) {
    server_.childPipeline(std::make_shared<BookiePipelineFactory>(*this));
}

void Bookie::start() {
    SocketAddress bookieAddress("0.0.0.0", conf_.bookiePort());
    LOG_INFO("Starting bookie on " << bookieAddress);
    server_.bind(bookieAddress);

    zk_.startSession();
    LOG_INFO("Started bookie");
}

void Bookie::stop() {
    server_.stop();
}

void Bookie::waitForStop() {
    server_.waitForStop();
}

BookieHandler Bookie::newHandler() {
    return BookieHandler(*this);
}

Future<Unit> Bookie::addEntry(int64_t ledgerId, int64_t entryId, IOBufPtr data) {
    union Key {
        struct {
            int64_t ledgerId;
            int64_t entryId;
        };
        char data[0];
    };

    Key key;
    key.ledgerId = htonll(ledgerId);
    key.entryId = htonll(entryId);

    rocksdb::Slice keySlice(key.data, sizeof(key));
    rocksdb::Slice valueSlice((const char*) data->data(), data->length());

    return storage_.put(keySlice, valueSlice);
}

Future<IOBufPtr> Bookie::getLastEntry(int64_t ledgerId) {
}

Future<IOBufPtr> Bookie::readEntry(int64_t ledgerId, int64_t entryId) {

}
