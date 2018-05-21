#pragma once

#include <stdint.h>
#include <sys/time.h>
#include <mutex>

const static int kDefaultMaxCount = 10000;
const static int kSec2Usec = 1000000;
class RateLimiter {
public:
    RateLimiter() {
        maxCount_ = kDefaultMaxCount;
        curCount_ = 0;
    }
    // interval is Microsecond
    // Second = 1000 * Millisecond = 1000 * 1000 * Microsecond
    // The effective rate limit is equal to maxCount/interval.
    // For example, if you want to a max QPS of 5000,
    // and want to limit bursts to no more than 500, you'd specify a maxCount of 500
    // and an interval of 100*Millilsecond = 100000.
    RateLimiter(int maxCount, int64_t interval) {
        maxCount_ = maxCount;
        interval_ = interval;
        lastTimeUsecs_ = 0;
        curCount_ = 0;
    }
    ~RateLimiter() {}

    // Allow returns true if a request is within the rate limit norms.
    // Otherwise, it returns false.
    bool Allow() {
        std::unique_lock<std::mutex> lock(mutex_);
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        int64_t now_usec = tv.tv_sec * kSec2Usec + tv.tv_usec;
        if (now_usec - lastTimeUsecs_ < interval_) {
            if (curCount_ > 0) {
                curCount_--;
                return true;
            }
            return false;
        }
        curCount_ = maxCount_ - 1;
        lastTimeUsecs_ = now_usec;
        return true;
    }
private:
    int maxCount_;
    int64_t interval_;
    int curCount_;
    int64_t lastTimeUsecs_;
    std::mutex mutex_;
};