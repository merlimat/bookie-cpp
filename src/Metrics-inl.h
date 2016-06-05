#pragma once

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

inline void Metric::addLatencySample(microseconds latency) {
    histogram_->addValue(latency.count());
}
