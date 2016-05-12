#include "Bookie.h"
#include "BookieConfig.h"
#include "Logging.h"

#include <glog/logging.h>
#include <gflags/gflags.h>

#include <folly/ThreadName.h>

#include <csignal>
#include <memory>

DECLARE_LOG_OBJECT();

std::unique_ptr<Bookie> bookie;

static void signalHandler(int signal) {
    LOG_INFO("Received signal " << signal << " - Shutting down");
    if (bookie) {
        bookie->stop();
    }
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGQUIT, signalHandler);

    Logging::init();
    google::InitGoogleLogging(argv[0]);

    BookieConfig config;
    if (!config.parse(argc, argv)) {
        return -1;
    }

    bookie = make_unique<Bookie>(config);
    bookie->start();
    bookie->waitForStop();

    // Trigger bookie destructor
    bookie.reset(nullptr);

    return 0;
}
