#pragma once

#include <chrono>
#include <thread>

class JournalRateLimiter {
 public:
    JournalRateLimiter(double rate);

    void aquire();

    void aquire(int permits);

 private:
    JournalRateLimiter(const JournalRateLimiter&);
    JournalRateLimiter& operator=(const JournalRateLimiter&);
    typedef std::chrono::high_resolution_clock Clock;
    Clock::duration interval_;

    long storedPermits_;
    double maxPermits_;
    Clock::time_point nextFree_;
};

JournalRateLimiter::JournalRateLimiter(double rate)
        : interval_(std::chrono::microseconds((long)(1e6 / rate))),
          storedPermits_(0.0),
          maxPermits_(rate),
          nextFree_() {
    assert(rate < 1e6 && "Exceeded maximum rate");
}

void JournalRateLimiter::aquire() {
    aquire(1);
}

void JournalRateLimiter::aquire(int permits) {
    Clock::time_point now = Clock::now();

    if (now > nextFree_) {
        storedPermits_ = std::min<long>(maxPermits_,
                                        storedPermits_ + (now - nextFree_) / interval_);
        nextFree_ = now;
    }

    Clock::duration wait = nextFree_ - now;

    // Determine how many stored and fresh permits to consume
    long stored = std::min<long>(permits, storedPermits_);
    long fresh = permits - stored;

    // In the general RateLimiter, stored permits have no wait time,
    // and thus we only have to wait for however many fresh permits we consume
    Clock::duration next = fresh * interval_;
    nextFree_ += next;
    storedPermits_ -= stored;

    if (wait != Clock::duration::zero()) {
        std::this_thread::sleep_for(wait);
    }
}
