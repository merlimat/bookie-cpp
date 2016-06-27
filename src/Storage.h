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

#include <rocksdb/db.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>
#include <folly/MPMCQueue.h>

#include <memory>
#include <thread>

#include "BookieConfig.h"
#include "Metrics.h"

using namespace folly;
using rocksdb::Slice;
typedef std::unique_ptr<IOBuf> IOBufPtr;

class Storage {
public:
    Storage(const BookieConfig& conf, MetricsManager& metricsManager);
    ~Storage();

    Future<Unit> put(int64_t ledgerId, int64_t entryId, IOBufPtr data);

private:
    void runJournal();

    rocksdb::DB* db_;
    const rocksdb::WriteOptions writeOptions_;

    typedef std::unique_ptr<Promise<Unit>> PromisePtr;

    struct JournalEntry {
        Slice key;
        IOBufPtr data;
        PromisePtr promise;
        Timer walTimeSpentInQueue;
    };

    MPMCQueue<JournalEntry> journalQueue_;

    const bool fsyncWal_;
    std::thread journalThread_;

    MetricPtr rocksDbPutLatency_;
    MetricPtr addEntryEnqueueLatency_;
    MetricPtr walSyncLatency_;
    MetricPtr walQueueLatency_;
};

