#pragma once

#include <wangle/bootstrap/ServerBootstrap.h>
#include <iostream>

#include "BookiePipeline.h"
#include "BookieRegistration.h"
#include "ZooKeeper.h"
#include "BookieHandler.h"
#include "BookieConfig.h"
#include "Storage.h"

using namespace wangle;

class Bookie {
public:
    explicit Bookie(const BookieConfig& conf);

    void start();

    void stop();

    void waitForStop();

    BookieHandler newHandler();

    Future<Unit> addEntry(int64_t ledgerId, int64_t entryId, IOBufPtr data);

    Future<IOBufPtr> getLastEntry(int64_t ledgerId);

    Future<IOBufPtr> readEntry(int64_t ledgerId, int64_t entryId);

private:
    const BookieConfig& conf_;
    ServerBootstrap<BookiePipeline> server_;

    ZooKeeper zk_;
    BookieRegistration bookieRegistration_;
    Storage storage_;
};

