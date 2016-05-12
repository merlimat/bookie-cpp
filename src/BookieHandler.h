#pragma once

#include "BookieProtocol.h"

#include <folly/SocketAddress.h>
#include <wangle/channel/Handler.h>

using namespace wangle;
using namespace folly;

class Bookie;

class BookieHandler: public HandlerAdapter<Request, Response> {
public:
    BookieHandler(Bookie& bookie);

    virtual void transportActive(Context* ctx) override;

    virtual void readEOF(Context* ctx) override;

    virtual void read(Context* ctx, Request request) override;

private:
    void handleAddEntry(Context* ctx, Request request);
    void handleReadEntry(Context* ctx, Request request);

    Bookie& bookie_;
    SocketAddress peerAddress_;
};
