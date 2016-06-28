/**
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
#pragma once

#include <wangle/bootstrap/ServerBootstrap.h>
#include <iostream>

#include "BookiePipeline.h"
#include "BookieRegistration.h"
#include "ZooKeeper.h"
#include "BookieHandler.h"
#include "BookieConfig.h"
#include "Metrics.h"
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
    MetricsManager metricsManager_;
    ServerBootstrap<BookiePipeline> server_;

    ZooKeeper zk_;
    BookieRegistration bookieRegistration_;
    Storage storage_;
};

