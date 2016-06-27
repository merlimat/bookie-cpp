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

#include "Logging.h"
#include "Metrics.h"

#include <folly/json.h>
#include <folly/ThreadName.h>

DECLARE_LOG_OBJECT();

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
        hist.clear();
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
    json::serialization_opts opts;
    opts.pretty_formatting = false;
    opts.sort_keys = true;


    std::lock_guard<std::mutex> lock(mutex_);

    LOG_INFO("--- Stats ---");
    for (auto& metric : metrics_) {
        metric.second->updateStats(statsPeriod_);

        LOG_INFO(metric.first << " : " << json::serialize(metric.second->getStats(), opts));
    }

    // Schedule next stats update
    eventBase_.runAfterDelay(std::bind(&MetricsManager::updateStats, this), milliseconds(statsPeriod_).count());
}

std::string MetricsManager::getJsonStats(bool formatJson) {
    std::lock_guard<std::mutex> lock(mutex_);
    return getJsonStatsNoLock(formatJson);
}

std::string MetricsManager::getJsonStatsNoLock(bool formatJson) {
    dynamic stats = dynamic::object();
    for (auto& metric : metrics_) {
        stats[metric.first] = metric.second->getStats();
    }

    json::serialization_opts opts;
    opts.pretty_formatting = formatJson;
    opts.sort_keys = true;
    return json::serialize(stats, opts);
}
