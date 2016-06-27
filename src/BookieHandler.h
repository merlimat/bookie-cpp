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

#include "BookieProtocol.h"
#include "Metrics.h"

#include <folly/SocketAddress.h>
#include <wangle/channel/Handler.h>

using namespace wangle;
using namespace folly;

class Bookie;

class BookieHandler: public HandlerAdapter<Request, Response> {
public:
    BookieHandler(Bookie& bookie, MetricsManager& metricsManager);

    virtual void transportActive(Context* ctx) override;

    virtual void readEOF(Context* ctx) override;

    virtual void read(Context* ctx, Request request) override;

private:
    void handleAddEntry(Context* ctx, Request request);
    void handleReadEntry(Context* ctx, Request request);

    Bookie& bookie_;
    SocketAddress peerAddress_;

    MetricPtr addEntryLatency_;
};
