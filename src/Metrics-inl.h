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

inline Timer::Timer() :
        metric_(nullptr),
        startTime_() {
}

inline Timer::Timer(Metric* metric) :
        metric_(metric),
        startTime_(Clock::now()) {
}

inline void Timer::completed() {
    metric_->addLatencySample(Clock::now() - startTime_);
}

inline const std::string& Metric::name() const {
    return name_;
}

inline Timer Metric::startTimer() {
    return Timer(this);
}

inline void Metric::addLatencySample(Clock::duration latency) {
    histogram_->addValue(duration_cast<microseconds>(latency).count());
}

inline void Metric::addValueSample(uint64_t value) {
    // Multiply value since it's expecting to get a "micros" latency
    // that will be later presented in millis
    histogram_->addValue(value * 1000);
}
