#include <thread>

#include "Logging.h"
#include "Metrics.h"

DECLARE_LOG_OBJECT();

int main(int argc, char** argv) {
    Logging::init();

    MetricsManager metricsManager(seconds(2));
    MetricPtr metric = metricsManager.createMetric("test-metric");

    for (int i = 0; i < 10000; i++) {
        Timer timer = metric->startTimer();
        std::this_thread::sleep_for(microseconds(10));
        timer.completed();
    }

    std::this_thread::sleep_for(seconds(2));
    LOG_INFO("Stats: " << metricsManager.getJsonStats());
}
