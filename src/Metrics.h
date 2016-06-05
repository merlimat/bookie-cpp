#pragma once

#include <chrono>
#include <string>
#include <map>

#include <folly/dynamic.h>
#include <folly/io/async/EventBase.h>
#include <folly/stats/Histogram.h>
#include <folly/ThreadLocal.h>

using namespace std::chrono;
using namespace folly;

typedef system_clock Clock;
typedef Clock::time_point TimePoint;

typedef Histogram<int64_t> LatencyHistogram;

class Metric;

/**
 * A one-time use timer to track the time taken for a certain operation
 */
class Timer {
public:
    void completed();

private:
    Timer(Metric* metric);

    Metric* metric_;
    TimePoint startTime_;

    friend class Metric;
};

typedef std::shared_ptr<Metric> MetricPtr;

class Metric {
public:
    Metric(const std::string& name);

    Timer startTimer();

    const std::string& name() const;

private:
    dynamic getStats();

    void addLatencySample(microseconds latency);
    void updateStats(seconds statsPeriod);

    const std::string name_;

    class HistogramTag;
    ThreadLocal<LatencyHistogram, HistogramTag> histogram_;

    dynamic stats_;

    friend class Timer;
    friend class MetricsManager;
};

typedef std::shared_ptr<Metric> MetricPtr;

class MetricsManager {
public:
    MetricsManager(seconds statsPeriod);
    ~MetricsManager();

    MetricPtr createMetric(const std::string& name);

    std::string getJsonStats();

private:
    void updateStats();

    std::map<std::string, MetricPtr> metrics_;
    seconds statsPeriod_;
    EventBase eventBase_;
    std::thread statsUpdateThread_;
    std::mutex mutex_;
};

#include "Metrics-inl.h"
