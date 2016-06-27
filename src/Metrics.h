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
    Timer();
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

    void addLatencySample(Clock::duration latency);
    void addValueSample(uint64_t value);

    const std::string& name() const;

private:
    dynamic getStats();


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

    std::string getJsonStats(bool formatJson = true);

private:
    void updateStats();
    std::string getJsonStatsNoLock(bool formatJson);

    std::map<std::string, MetricPtr> metrics_;
    seconds statsPeriod_;
    EventBase eventBase_;
    std::thread statsUpdateThread_;
    std::mutex mutex_;
};

#include "Metrics-inl.h"
