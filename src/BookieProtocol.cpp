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
#include "BookieProtocol.h"
#include <folly/Format.h>
#include <iostream>

using namespace folly;

std::ostream& operator<<(std::ostream& s, BookieOperation op) {
    switch (op) {
    case BookieOperation::AddEntry:
        s << "AddEntry";
        break;
    case BookieOperation::ReadEntry:
        s << "ReadEntry";
        break;
    case BookieOperation::Auth:
        s << "Auth";
        break;
    default:
        s << "Unknown bookie op (" << (int) op << ")";
        break;
    }

    return s;
}

std::ostream& operator<<(std::ostream& s, BookieError error) {
    switch (error) {
    case BookieError::OK:
        s << "OK";
        break;
    case BookieError::NoLedger:
        s << "NoLedger";
        break;
    case BookieError::NoEntry:
        s << "NoEntry";
        break;
    case BookieError::BadRequest:
        s << "BadRequest";
        break;
    case BookieError::IOError:
        s << "IOError";
        break;
    case BookieError::UnauthorizedAccesss:
        s << "UnauthorizedAccess";
        break;
    case BookieError::BadVersion:
        s << "BadVersion";
        break;
    case BookieError::Fenced:
        s << "Fenced";
        break;
    case BookieError::ReadOnly:
        s << "ReadOnly";
        break;
    case BookieError::TooManyRequests:
        s << "TooManyRequests";
        break;
    }

    return s;
}

std::ostream& operator<<(std::ostream& s, const Request& r) {
    s << "Request(" //
            << "version:" << (int) r.protocolVersion //
            << " opCode:" << r.opCode //
            << " ledgerId:" << r.ledgerId //
            << " entryId:" << r.entryId //
            << " flags:0x" << format("{0:04x}", r.flags) //
            << " data-len:" << (r.data ? r.data->length() : 0) //
            << ")";

    return s;
}

std::ostream& operator<<(std::ostream& s, const Response& r) {
    s << "Response(" //
            << "version:" << (int) r.protocolVersion //
            << " opCode:" << r.opCode //
            << " error:" << r.errorCode //
            << " ledgerId:" << r.ledgerId //
            << " entryId:" << r.entryId //
            << " data-len:" << (r.data ? r.data->length() : 0) //
            << ")";

    return s;
}

