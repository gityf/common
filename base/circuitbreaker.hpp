/*
** Copyright (C) 2019 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of CircuitBreaker.
*/

#pragma once

#include <stdint.h>
#include <sys/time.h>
#include <atomic>

/**
 * usage:
 *
 * CircuitBreaker cb;
 * if (cb.allow()) {
 *    if (callServiceSucc()) {
 *      cb.feedSuccess();
 *    } else {
 *      cb.feedFail();
 *    }
 * }
 */
class CircuitBreaker {
public:
    const static int kSec2Usec = 1000000;
    const static int kCalcStatIntervalUs = 1 * kSec2Usec;
    const static int kCircuitOpen = 0;
    const static int kCircuitHalfOpen = 1;
    const static int kCircuitClose = 2;

    CircuitBreaker() {
        enabled_ = false;
        forceOpen_ = false;
        forceClose_ = false;
        lastCalcStatTimeUsecs_ = 0;
        sleepWindowsInUs_ = 2000000;
        lastCircuitOpenTime_ = 0;
        circuitStatus_ = kCircuitClose;
        successCount_ = 0;
        failCount_ = 0;
        errorThresholdPercentage_ = 0.5;
    }
    ~CircuitBreaker() {}

    // set percentage of error threshold.
    void setErrorThresholdPercentage(double percentage = 0.5) {
        errorThresholdPercentage_ = percentage;
    }

    // Allow returns true if a request is within the circuit breaker norms.
    // Otherwise, it returns false.
    bool allow() {
        // force open the circuit, link is break so this is not allowed.
        if (forceOpen_) {
            return false;
        }
        // force close the circuit, link is not break so this is allowed.
        if (forceClose_) {
            return true;
        }

        int64_t now_usec = getNowUs();
        calcStat(now_usec);

        if (circuitStatus_ == kCircuitClose) {
            return true;
        } else {
            if (isAfterSleepWindow(now_usec)) {
                lastCircuitOpenTime_ = now_usec;
                circuitStatus_ = kCircuitHalfOpen;
                // sleep so long time, try ones, and set status to half-open;
                return true;
            }
        }
        return false;
    }

    // call me when the service called failed.
    void feedFail () {
        failCount_++;
    }

    // call me when the service called success.
    void feedSuccess() {
        successCount_++;
    }

    void setForceOpen() {
        forceOpen_ = true;
        forceClose_ = false;
    }

    void setForceClose() {
        forceClose_ = true;
        forceOpen_ = false;
    }

    void calcStat(int64_t now_usec) {
        if (now_usec > lastCalcStatTimeUsecs_ + kCalcStatIntervalUs) {
            double rate = (double)failCount_ / (double)(successCount_ + failCount_);
            if (failCount_ > 0 && rate >= errorThresholdPercentage_) {
                markNonSuccess();
                lastCircuitOpenTime_ = now_usec;
            } else {
                markSuccess();
            }
            // clear count
            successCount_ = 0;
            failCount_ = 0;
            lastCalcStatTimeUsecs_ = now_usec;
        }
    }

private:
    void markNonSuccess() {
        // mark CLOSE to OPEN
        circuitStatus_ = kCircuitOpen;
    }

    void markSuccess() {
        lastCircuitOpenTime_ = 0;
        // mark OPEN to CLOSE
        circuitStatus_ = kCircuitClose;
    }

    // check whether after the sleep windows
    bool isAfterSleepWindow(int64_t now_usec) {
        return (now_usec > lastCircuitOpenTime_ + sleepWindowsInUs_) ? true : false;
    }

    int64_t getNowUs() {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        int64_t now_usec = tv.tv_sec * kSec2Usec + tv.tv_usec;
        return now_usec;
    }

private:
    // CircuitBreaker enable or not
    bool enabled_;
    // force open circuit breaker
    bool forceOpen_;
    // force close curuit breaker
    bool forceClose_;
    // error threshold percentage, default 50%
    double errorThresholdPercentage_;
    // how long time 1ms=1000us if circuit is break. default 2000ms
    int sleepWindowsInUs_;
    int64_t lastCalcStatTimeUsecs_;
    int64_t lastCircuitOpenTime_;

    std::atomic<int64_t> successCount_;
    std::atomic<int64_t> failCount_;
    std::atomic<int> circuitStatus_;
};