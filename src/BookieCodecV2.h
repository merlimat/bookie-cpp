#pragma once

#include <wangle/channel/Handler.h>

#include "BookieProtocol.h"

using namespace wangle;
using namespace folly;

/*
 * Codec for BookKeeper V2 wire format
 */
class BookieCodecV2: public Handler<IOBufPtr, Request, Response, IOBufPtr> {
public:
    void read(Context* ctx, IOBufPtr buf) override;

    Future<Unit> write(Context* ctx, Response response) override;
};

