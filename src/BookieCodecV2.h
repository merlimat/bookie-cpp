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

#include <wangle/channel/Handler.h>

#include "BookieProtocol.h"

using namespace wangle;
using namespace folly;

/**
 * Codec for BookKeeper V2 wire format
 */
class BookieServerCodecV2: public Handler<IOBufPtr, Request, Response, IOBufPtr> {
public:
    void read(Context* ctx, IOBufPtr buf) override;

    Future<Unit> write(Context* ctx, Response response) override;
};

/**
 * Codec for BookKeeper V2 wire format
 */
class BookieClientCodecV2: public Handler<IOBufPtr, Response, Request, IOBufPtr> {
public:
    void read(Context* ctx, IOBufPtr buf) override;

    Future<Unit> write(Context* ctx, Request response) override;
};
