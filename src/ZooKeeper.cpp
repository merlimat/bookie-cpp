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
#include "ZooKeeper.h"

#include "Logging.h"
#include <zookeeper/zookeeper.h>
#include <functional>
#include <memory>
#include <cassert>

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/futures/Future.h>

DECLARE_LOG_OBJECT();

template<typename T>
struct Context {
    ZooKeeper* client;
    std::string path;
    Promise<T> promise;
};

ZooKeeper::ZooKeeper(const std::string& zkServers, std::chrono::milliseconds sessionTimeout) :
        zkServers_(zkServers),
        sessionTimeout_(sessionTimeout),
        zk_(nullptr) {
}

ZooKeeper::~ZooKeeper() {
    if (zk_) {
        LOG_INFO("Closing zk session " << format("0x{0:x}", zoo_client_id(zk_)->client_id));
        int rc = zookeeper_close(zk_);
        if (rc != ZOK) {
            ZooKeeperError err = ZooKeeperException::getError(rc);
            LOG_WARN("Failed to close zk session: " << ZooKeeperException::getErrorMsg(err));
        }
    }
}

Future<uint64_t> ZooKeeper::startSession() {
    std::lock_guard<std::mutex> lock { mutex_ };

    LOG_INFO("Creating ZooKeeper session");
    zk_ = zookeeper_init(zkServers_.c_str(), &ZooKeeper::handleSessionEvent, sessionTimeout_.count(), nullptr, this, 0);
    return sessionPromise_.getFuture();
}

void ZooKeeper::handleSessionEvent(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx) {
    ZooKeeper* client = reinterpret_cast<ZooKeeper*>(watcherCtx);
    std::unique_lock<std::mutex> lock { client->mutex_ };

    LOG_INFO("Received ZK watch event. Type: " << getEventTypeStr(type) //
            << " -- State: " << getSessionStateStr(state)//
            << " -- path: '" << path << "'");

    if (state == ZOO_CONNECTED_STATE) {
        client->zk_ = zh;
        if (!client->sessionPromise_.isFulfilled()) {
            int64_t zkClientId = zoo_client_id(zh)->client_id;
            LOG_INFO("Established new ZooKeeper session " << format("0x{0:x}", zkClientId));
            client->sessionPromise_.setValue(zkClientId);

            // Notify listeners that a new session is ready
            auto toNotify { client->sessionListeners_ };
            lock.unlock();

            for (auto& listener : toNotify) {
                listener();
            }
        }
    } else if (state == ZOO_EXPIRED_SESSION_STATE) {
        LOG_WARN("ZooKeeper session expired");
        client->sessionPromise_ = {};

        lock.unlock();
        client->startSession();
    }
}

void ZooKeeper::registerSessionListener(SessionListener listener) {
    std::unique_lock<std::mutex> lock { mutex_ };

    sessionListeners_.push_back(listener);
    if (sessionPromise_.isFulfilled()) {
        // Session is already ready, notify immediately
        lock.unlock();

        listener();
    }
}

Future<std::string> ZooKeeper::create(const std::string& path, const std::string& value,
        std::initializer_list<CreateFlag> createFlags) {
    std::lock_guard<std::mutex> lock { mutex_ };

    Context<std::string>* ctx = new Context<std::string> { this, path };
    int flags = 0;
    for (auto flag : createFlags) {
        flags |= flag;
    }

    int rc = zoo_acreate(zk_, path.c_str(), value.c_str(), value.length(), &ZOO_OPEN_ACL_UNSAFE, flags,
            [](int rc, const char* path, const void* zkCtx) {
                Context<std::string>* ctx = (Context<std::string>*)zkCtx;

                if (rc == ZOK) {
                    LOG_DEBUG("Successfully created z-node at " << path);
                    ctx->promise.setValue(path);
                } else {
                    ctx->promise.setException(make_exception_wrapper<ZooKeeperException>(rc, //
                                    to<std::string>("Failed to create z-node at ", ctx->path) ));
                }

                delete ctx;
            }, ctx);

    if (rc != ZOK) {
        return makeFuture<std::string>(
                make_exception_wrapper<ZooKeeperException>(rc, to<std::string>("Failed to create z-node at ", path)));
    }

    return ctx->promise.getFuture();
}

std::string ZooKeeper::getEventTypeStr(int type) {
    switch (type) {
    case -1:
        return "None";
    case 1:
        return "NodeCreated";
    case 2:
        return "NodeDeleted";
    case 3:
        return "NodeDataChanged";
    case 4:
        return "NodeChildrenChanged";
    default:
        return to<std::string>(type);
    }
}

std::string ZooKeeper::getSessionStateStr(int state) {
    switch (state) {
    case 0:
        return "Disconnected";
    case 1:
        return "NoSyncConnected";
    case 3:
        return "Connected";
    case 5:
        return "ConnectedReadOnly";
    case 6:
        return "SaslAuthenticated";
    case -112:
        return "Expired";
    default:
        return to<std::string>(state);
    }
}

ZooKeeperException::ZooKeeperException(int rc) :
        zkError_(getError(rc)),
        msg_(getErrorMsg(zkError_)) {
}

ZooKeeperException::ZooKeeperException(int rc, const std::string& msg) :
        zkError_(getError(rc)),
        msg_(to<std::string>(msg, " - ", getErrorMsg(zkError_))) {
}

const char* ZooKeeperException::what() const noexcept {
    return msg_.c_str();
}

ZooKeeperError ZooKeeperException::error() const {
    return zkError_;
}

ZooKeeperError ZooKeeperException::getError(int rc) {
    return static_cast<ZooKeeperError>(rc);
}

std::string ZooKeeperException::getErrorMsg(ZooKeeperError zkError) {
    switch (zkError) {
    case ZooKeeperError::OK:
        return "OK";
    case ZooKeeperError::SystemError:
        return "System Error";
    case ZooKeeperError::RuntimeInconsistency:
        return "Runtime inconsistency";
    case ZooKeeperError::DataIinconsistency:
        return "Data inconsistency";
    case ZooKeeperError::ConnectionLoss:
        return "Connection loss";
    case ZooKeeperError::MarshallingError:
        return "Marshalling error";
    case ZooKeeperError::Unimplemented:
        return "Unimplemented";
    case ZooKeeperError::OperationTimeout:
        return "Operation timeout";
    case ZooKeeperError::BadArguments:
        return "Bad arguments";
    case ZooKeeperError::InvalidState:
        return "Invalid state";
    case ZooKeeperError::ApiError:
        return "API error";
    case ZooKeeperError::NoNode:
        return "No node";
    case ZooKeeperError::NoAuth:
        return "No Auth";
    case ZooKeeperError::BadVersion:
        return "Bad Version";
    case ZooKeeperError::NoChildrenForEphemerals:
        return "No children for ephemerals";
    case ZooKeeperError::NodeExists:
        return "Node exists";
    case ZooKeeperError::NotEmpty:
        return "Not empty";
    case ZooKeeperError::SessionExpired:
        return "Session expired";
    case ZooKeeperError::InvalidCallback:
        return "Invalid callback";
    case ZooKeeperError::InvalidACL:
        return "Invalid ACL";
    case ZooKeeperError::AuthFailed:
        return "Auth failed";
    case ZooKeeperError::Closing:
        return "Closing";
    case ZooKeeperError::Nothing:
        return "Nothing";
    case ZooKeeperError::SessionMoved:
        return "Session moved";
    default:
        return to<std::string>("Unknown error (", zkError, ")");
    }
}
