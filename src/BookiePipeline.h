#pragma once

#include <wangle/bootstrap/ServerBootstrap.h>

#include "BookieProtocol.h"

using namespace wangle;
using namespace folly;

typedef Pipeline<IOBufQueue&, Request> BookiePipeline;

class Bookie;

/**
 * Define the processing pipeline for serialize/deserialize bookie commands
 */
class BookiePipelineFactory: public PipelineFactory<BookiePipeline> {
public:
    BookiePipelineFactory(Bookie& bookie);

    BookiePipeline::Ptr newPipeline(std::shared_ptr<AsyncTransportWrapper> sock) override;

private:
    Bookie& bookie_;
};
