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
