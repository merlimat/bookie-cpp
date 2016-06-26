#include "Bookie.h"
#include "Logging.h"

#include <folly/io/async/EventBaseManager.h>
#include <wangle/channel/EventBaseHandler.h>
#include <folly/Bits.h>

DECLARE_LOG_OBJECT();

Bookie::Bookie(const BookieConfig& conf) :
        conf_(conf),
        metricsManager_(conf.statsReportingInterval()),
        zk_(conf.zkServers(), milliseconds(conf.zkSessionTimeout())),
        bookieRegistration_(&zk_, conf),
        storage_(conf, metricsManager_) {
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
    return BookieHandler(*this, metricsManager_);
}

Future<Unit> Bookie::addEntry(int64_t ledgerId, int64_t entryId, IOBufPtr data) {
    return storage_.put(ledgerId, entryId, std::move(data));
}

Future<IOBufPtr> Bookie::getLastEntry(int64_t ledgerId) {
}

Future<IOBufPtr> Bookie::readEntry(int64_t ledgerId, int64_t entryId) {

}
