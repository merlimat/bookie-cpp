#include <thread>

#include "Logging.h"
#include "Metrics.h"
#include "BookieCodecV2.h"
#include "RateLimiter.h"
#include <iostream>

#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/channel/EventBaseHandler.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

DECLARE_LOG_OBJECT();

struct Arguments {
    std::string bookieAddress;
    double rate;
    int msgSize;
    int numberOfConnections;
};

typedef Pipeline<IOBufQueue&, Request> BookieClientPipeline;
typedef ClientBootstrap<BookieClientPipeline> Client;

class AddEntryTask: public HandlerAdapter<Response, Request> {
public:
    AddEntryTask(BookieClientPipeline::Ptr pipeline, double rate, int msgSize, MetricPtr addEntryMetric) :
            pipeline_(pipeline),
            rateLimiter_(rate),
            msgSize_(msgSize),
            addEntryMetric_(addEntryMetric),
            thread_() {
        LOG_INFO("Created task");
    }

    void start() {
        LOG_INFO("Started add entry task " << bookieAddress_);
        int64_t ledgerId = ledgerIdGenerator_++;
        int64_t entryIdGenerator = 0;

        std::string payload;
        payload.resize(msgSize_);

        EventBase* eventBase = pipeline_->getTransport()->getEventBase();

        while (true) {
            rateLimiter_.aquire();

            int64_t entryId = entryIdGenerator++;

            eventBase->runInEventBaseThread([&]() {
                Request request {2, BookieOperation::AddEntry, ledgerId, entryId, 0, IOBuf::wrapBuffer(payload.c_str(),
                            payload.length())};
                pipeline_->write(std::move(request));
            });
        }
    }

    virtual void transportActive(Context* ctx) override {
        LOG_INFO("Transport active");
        ctx->fireTransportActive();
        ctx->getTransport()->getPeerAddress(&bookieAddress_);
        thread_ = std::make_unique<std::thread>(std::bind(&AddEntryTask::start, this));
    }

    virtual void read(Context* ctx, Response response) override {
        LOG_INFO("Received response: " << response);
    }

    virtual void readEOF(Context* ctx) override {
        std::cout << "EOF received" << std::endl;
        close(ctx);
    }

private:
    BookieClientPipeline::Ptr pipeline_;
    SocketAddress bookieAddress_;
    RateLimiter rateLimiter_;
    int msgSize_;
    MetricPtr addEntryMetric_;
    std::unique_ptr<std::thread> thread_;

    std::unordered_map<int64_t, TimePoint> pendingRequests_;

    static std::atomic<int64_t> ledgerIdGenerator_;
};

std::atomic<int64_t> AddEntryTask::ledgerIdGenerator_;

class BookieClientPipelineFactory: public PipelineFactory<BookieClientPipeline> {
    double perConnectionRate_;
    int msgSize_;
    MetricPtr addEntryMetric_;

public:

    BookieClientPipelineFactory(double rate, int msgSize, MetricPtr addEntryMetric) :
            perConnectionRate_(rate),
            msgSize_(msgSize),
            addEntryMetric_(addEntryMetric) {
    }

    BookieClientPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransportWrapper> sock) {
        auto pipeline = BookieClientPipeline::create();
        pipeline->addBack(AsyncSocketHandler(sock));
        pipeline->addBack(LengthFieldBasedFrameDecoder(4, BookieConstant::MaxFrameSize));
        pipeline->addBack(LengthFieldPrepender());
        pipeline->addBack(BookieClientCodecV2());
        pipeline->addBack(std::make_shared<AddEntryTask>(pipeline, perConnectionRate_, msgSize_, addEntryMetric_));
        pipeline->finalize();
        return pipeline;
    }
};

int main(int argc, char** argv) {
    Logging::init();

    Arguments args;

    po::options_description options;
    options.add_options() //
    ("help,h", "This help message") //
    ("bookieAddress,a", po::value<std::string>(&args.bookieAddress)->default_value("localhost:3181"),
            "Boookie hostname and port") //
    ("rate,r", po::value<double>(&args.rate)->default_value(100), "Add entry rate") //
    ("msg-size,s", po::value<int>(&args.msgSize)->default_value(1024), "Message size") //
    ("num-connections,c", po::value<int>(&args.numberOfConnections)->default_value(16), "Number of connections") //
            ;

    po::variables_map map;
    try {
        po::store(po::command_line_parser(argc, argv).options(options).run(), map);
        po::notify(map);

        if (map.count("help")) {
            std::cerr << options << std::endl;
            exit(1);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing parameters -- " << e.what() << std::endl << std::endl;
        std::cerr << options << std::endl;
        return -1;
    }

    MetricsManager metricsManager(seconds(2));
    MetricPtr addEntryMetric = metricsManager.createMetric("add-entry-metric");

    SocketAddress bookieAddress;
    bookieAddress.setFromHostPort(args.bookieAddress);
    LOG_INFO("Bookie address: " << bookieAddress);

//    std::vector<std::unique_ptr<AddEntryTask>> tasks;

    double perConnectionRate = args.rate / args.numberOfConnections;

    ClientBootstrap<BookieClientPipeline> client;
    client.group(std::make_shared<wangle::IOThreadPoolExecutor>(16));
    client.pipelineFactory(
            std::make_shared<BookieClientPipelineFactory>(perConnectionRate, args.msgSize, addEntryMetric));

    for (int i = 0; i < args.numberOfConnections; i++) {
        client.connect(bookieAddress).get();
//        tasks.push_back(
//                std::make_unique<AddEntryTask>(&client, bookieAddress, perConnectionRate, args.msgSize,
//                        addEntryMetric));
    }

    while (true) {
        std::this_thread::sleep_for(seconds(1));
    }
}
