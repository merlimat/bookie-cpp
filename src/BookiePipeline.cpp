#include "BookiePipeline.h"
#include "BookieCodecV2.h"
#include "Bookie.h"

#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>

enum {
    MaxFrameSize = 5 * 1024 * 1024
};

BookiePipelineFactory::BookiePipelineFactory(Bookie& bookie) :
        bookie_(bookie) {
}

BookiePipeline::Ptr BookiePipelineFactory::newPipeline(std::shared_ptr<AsyncTransportWrapper> sock) {
    auto pipeline = BookiePipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(LengthFieldBasedFrameDecoder(4, MaxFrameSize));
    pipeline->addBack(LengthFieldPrepender());
    pipeline->addBack(BookieCodecV2());
    pipeline->addBack(bookie_.newHandler());
    pipeline->finalize();
    return pipeline;
}
