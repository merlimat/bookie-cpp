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
