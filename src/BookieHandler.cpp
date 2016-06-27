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
#include "Logging.h"
#include "BookieHandler.h"
#include "Bookie.h"

DECLARE_LOG_OBJECT();

BookieHandler::BookieHandler(Bookie& bookie, MetricsManager& metricsManager) :
        bookie_(bookie),
        addEntryLatency_(metricsManager.createMetric("addEntry")) {
}

void BookieHandler::transportActive(Context* ctx) {
    ctx->getTransport()->getPeerAddress(&peerAddress_);
    LOG_INFO("New connection from " << peerAddress_);
    ctx->fireTransportActive();
}

void BookieHandler::readEOF(Context* ctx) {
    LOG_INFO("Closed connection from " << peerAddress_);
    ctx->fireReadEOF();
}

void BookieHandler::read(Context* ctx, Request request) {
    switch (request.opCode) {
    case BookieOperation::AddEntry:
        handleAddEntry(ctx, std::move(request));
        break;

    case BookieOperation::ReadEntry:
        handleReadEntry(ctx, std::move(request));
        break;

    }
}

void BookieHandler::handleAddEntry(Context* ctx, Request request) {
    int64_t ledgerId = request.ledgerId;
    int64_t entryId = request.entryId;
    uint64_t entryLength = request.data->length();

    Clock::time_point start = Clock::now();

    Future<Unit> future = bookie_.addEntry(request.ledgerId, request.entryId, std::move(request.data)); //
    future.then(ctx->getTransport()->getEventBase(), [=](Unit u) {
        LOG_DEBUG("Entry persisted at " << ledgerId << ":" << entryId << " -- size: " << entryLength);
        Response response {2, BookieOperation::AddEntry, BookieError::OK, ledgerId, entryId};

        write(ctx, std::move(response));

        addEntryLatency_->addLatencySample(Clock::now() - start);
    }) //
    .onError([=](const std::exception& e) {
        LOG_WARN("Failed to persist entry at " << ledgerId << ":" << entryId << " : " << e.what());
        Response response {2, BookieOperation::AddEntry, BookieError::IOError, ledgerId, entryId};

        write(ctx, std::move(response));
    });
}

void BookieHandler::handleReadEntry(Context* ctx, Request request) {

}
