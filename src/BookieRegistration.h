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

class ZooKeeper;
class BookieConfig;

#include <string>

/**
 * Register the bookie under /ledgers/available and makes sure the z-node is re-created after the session expires
 */
class BookieRegistration {
public:
    BookieRegistration(ZooKeeper* zk_, const BookieConfig& conf);

private:
    void handleNewZooKeeperSession();

    void registerBookie();

    ZooKeeper* zk_;
    std::string registrationPath_;
};
