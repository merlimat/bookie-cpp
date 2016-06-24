#include "Metrics.h"

#include <folly/json.h>
#include <folly/ThreadName.h>

static const int64_t BucketSize = 100;
static const int64_t MinValue = 0;
static const int64_t MaxValue = microseconds(seconds(1)).count();

Metric::Metric(const std::string& name) :
        name_(name),
        histogram_([]() {
            return new LatencyHistogram(BucketSize, MinValue, MaxValue);
        }),
        stats_(dynamic::object()) {
}

typedef duration<double, std::milli> double_millis;

inline double toMillis(int64_t micros) {
    return double_millis(microseconds(micros)).count();
}

dynamic Metric::getStats() {
    return stats_;
}

void Metric::updateStats(seconds statsPeriod) {
    LatencyHistogram aggregated(BucketSize, MinValue, MaxValue);
    for (LatencyHistogram& hist : histogram_.accessAllThreads()) {
        aggregated.merge(hist);
    }

    uint64_t count = 0;
    for (int i = 0; i < aggregated.getNumBuckets(); i++) {
        count += aggregated.getBucketByIndex(i).count;
    }

    double rate = count / (double) statsPeriod.count();

    stats_["min"] = toMillis(aggregated.getPercentileEstimate(0.0000));
    stats_["pct50"] = toMillis(aggregated.getPercentileEstimate(0.5000));
    stats_["pct75"] = toMillis(aggregated.getPercentileEstimate(0.7500));
    stats_["pct90"] = toMillis(aggregated.getPercentileEstimate(0.9000));
    stats_["pct95"] = toMillis(aggregated.getPercentileEstimate(0.9500));
    stats_["pct99"] = toMillis(aggregated.getPercentileEstimate(0.9900));
    stats_["pct999"] = toMillis(aggregated.getPercentileEstimate(0.9990));
    stats_["pct9999"] = toMillis(aggregated.getPercentileEstimate(0.9999));
    stats_["max"] = toMillis(aggregated.getPercentileEstimate(1.0000));
    stats_["count"] = count;
    stats_["rate"] = rate;
}

MetricsManager::MetricsManager(seconds statsPeriod) :
        statsPeriod_(statsPeriod),
        eventBase_(),
        statsUpdateThread_([=] {
            setThreadName("bookie-stats-updater");
            eventBase_.runAfterDelay(std::bind(&MetricsManager::updateStats, this), milliseconds(statsPeriod_).count());
            eventBase_.loopForever();
        }) {
}

MetricsManager::~MetricsManager() {
    eventBase_.terminateLoopSoon();
    statsUpdateThread_.join();
}

MetricPtr MetricsManager::createMetric(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = metrics_.find(name);
    if (it != metrics_.end()) {
        return it->second;
    }

    // Insert new metric
    MetricPtr metric = std::make_shared<Metric>(name);
    metrics_[name] = metric;
    return metric;
}

void MetricsManager::updateStats() {
    // Schedule next stats update
    eventBase_.runAfterDelay(std::bind(&MetricsManager::updateStats, this), milliseconds(statsPeriod_).count());

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& metric : metrics_) {
        metric.second->updateStats(statsPeriod_);
    }
}

std::string MetricsManager::getJsonStats() {
    std::lock_guard<std::mutex> lock(mutex_);

    dynamic stats = dynamic::object();
    for (auto& metric : metrics_) {
        stats[metric.first] = metric.second->getStats();
    }

    json::serialization_opts opts;
    opts.pretty_formatting = true;
    opts.sort_keys = true;
    return json::serialize(stats, opts);
}
