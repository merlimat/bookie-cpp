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

#include <folly/futures/Future.h>

struct _zhandle;

using namespace folly;

enum class ZooKeeperError
    : int {
        OK = 0, /*!< Everything is OK */

        /** System and server-side errors.
         * This is never thrown by the server, it shouldn't be used other than
         * to indicate a range. Specifically error codes greater than this
         * value, but lesser than {@link #ZAPIERROR}, are system errors. */
        SystemError = -1, //
    RuntimeInconsistency = -2, /*!< A runtime inconsistency was found */
    DataIinconsistency = -3, /*!< A data inconsistency was found */
    ConnectionLoss = -4, /*!< Connection to the server has been lost */
    MarshallingError = -5, /*!< Error while marshalling or unmarshalling data */
    Unimplemented = -6, /*!< Operation is unimplemented */
    OperationTimeout = -7, /*!< Operation timeout */
    BadArguments = -8, /*!< Invalid arguments */
    InvalidState = -9, /*!< Invliad zhandle state */

    /** API errors.
     * This is never thrown by the server, it shouldn't be used other than
     * to indicate a range. Specifically error codes greater than this
     * value are API errors (while values less than this indicate a
     * {@link #ZSYSTEMERROR}).
     */
    ApiError = -100,
    NoNode = -101, /*!< Node does not exist */
    NoAuth = -102, /*!< Not authenticated */
    BadVersion = -103, /*!< Version conflict */
    NoChildrenForEphemerals = -108, /*!< Ephemeral nodes may not have children */
    NodeExists = -110, /*!< The node already exists */
    NotEmpty = -111, /*!< The node has children */
    SessionExpired = -112, /*!< The session has been expired by the server */
    InvalidCallback = -113, /*!< Invalid callback specified */
    InvalidACL = -114, /*!< Invalid ACL specified */
    AuthFailed = -115, /*!< Client authentication failed */
    Closing = -116, /*!< ZooKeeper is closing */
    Nothing = -117, /*!< (not error) no server responses to process */
    SessionMoved = -118 /*!<session moved to another server, so operation is ignored */
};

class ZooKeeperException: public std::exception {
public:
    explicit ZooKeeperException(int rc);
    ZooKeeperException(int rc, const std::string& msg);
    const char* what() const noexcept override;

    ZooKeeperError error() const;

    static ZooKeeperError getError(int rc);
    static std::string getErrorMsg(ZooKeeperError zkError);

private:
    ZooKeeperError zkError_;
    std::string msg_;
};

class ZooKeeper {
public:
    typedef std::function<void()> SessionListener;

    enum CreateFlag {
        Ephemeral = 0x01, //
        Sequence = 0x02,
    };

    ZooKeeper(const std::string& zkServers, std::chrono::milliseconds sessionTimeout);
    ZooKeeper(const ZooKeeper& x) = delete;
    ZooKeeper& operator =(const ZooKeeper& x) = delete;
    ~ZooKeeper();

    Future<uint64_t> startSession();

    void registerSessionListener(SessionListener listener);

    /**
     * Create a z-node
     *
     * @param path
     * @param value
     * @param createFlags
     * @return a future yielding the effective path of the created z-node
     */
    Future<std::string> create(const std::string& path, const std::string& value,
            std::initializer_list<CreateFlag> createFlags);

private:
    static void handleSessionEvent(_zhandle* zh, int type, int state, const char* path, void* watcherCtx);

    static std::string getEventTypeStr(int type);
    static std::string getSessionStateStr(int state);

    std::mutex mutex_;
    std::string zkServers_;
    std::chrono::milliseconds sessionTimeout_;

    struct _zhandle* zk_;

    Promise<uint64_t> sessionPromise_;
    std::vector<SessionListener> sessionListeners_;
};
