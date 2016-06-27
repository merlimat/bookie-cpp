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

#include <chrono>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

using namespace std::chrono;

class BookieConfig {
public:
    BookieConfig();

    bool parse(int argc, char** argv);

    const std::string& zkServers() const {
        return zkServers_;
    }

    int zkSessionTimeout() const {
        return zkSessionTimeout_;
    }

    const std::string bookieHost() const {
        return bookieHost_;
    }

    int bookiePort() const {
        return bookiePort_;
    }

    const std::string& dataDirectory() const {
        return dataDirectory_;
    }

    const std::string& walDirectory() const {
        return walDirectory_;
    }

    bool fsyncWal() const {
        return fsyncWal_;
    }

    seconds statsReportingInterval() const {
        return seconds(statsReportingIntervalSeconds_);
    }

private:
    std::string zkServers_;
    int zkSessionTimeout_;

    std::string bookieHost_;
    int bookiePort_;

    std::string dataDirectory_;
    std::string walDirectory_;
    bool fsyncWal_;

    int statsReportingIntervalSeconds_;

    po::options_description options_;
};
