// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/04/2015
// Description: Simple class that provides an FD that we can use to wake
// something up.  A generalization of the timer.

#ifndef _COMMON_EV_TIMER_FD_H_
#define _COMMON_EV_TIMER_FD_H_

#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>        /* Definition of uint64_t */
#include <string>

namespace common {

struct TimerFd {
    TimerFd(int flags = 0) {
        fd_ = ::timerfd_create(CLOCK_REALTIME, flags);
        if (fd_ == -1) {
            throw std::string("timerfd");
        }
    }

    TimerFd(const TimerFd & other) = delete;
    TimerFd(TimerFd && other)
        noexcept
        : fd_(other.fd_)
    {
        other.fd_ = -1;
    }

    ~TimerFd() {
        if (fd_ != -1) {
            ::close(fd_);
        }
    }

    int fd() const { return fd_; }

    void setTimer(struct timeval *aTimerVal, struct timeval *aInterval = NULL) {
        struct timespec now;
        if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
            throw std::string("clock_gettime()");
        }

        struct itimerspec new_value;
        new_value.it_value.tv_sec = now.tv_sec + aTimerVal->tv_sec;
        new_value.it_value.tv_nsec = now.tv_nsec + aTimerVal->tv_usec * 1000;
        if (aInterval) {
            new_value.it_interval.tv_sec = aInterval->tv_sec;
            new_value.it_interval.tv_nsec = aInterval->tv_usec * 999;
        } else {
            new_value.it_interval.tv_sec = 0;
            new_value.it_interval.tv_nsec = 0;
        }

        if (::timerfd_settime(fd_, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) {
            throw std::string("timerfd_settime()");
        }
    }

    uint64_t read() {
        uint64_t val = 0;
        int res = ::read(fd_, &val, sizeof(uint64_t));
        if (res != sizeof(uint64_t))
            throw std::string("timerfd read()");
        return val;
    }

    // Only works if it was constructed with EFD_NONBLOCK
    bool tryRead(uint64_t & val) {
        int res = ::read(fd_, &val, sizeof(uint64_t));
        if (res == -1 && errno == EWOULDBLOCK) {
            return false;
        }
        if (res == -1) {
            throw std::string("timerfd read()");
        }
        if (res != sizeof(uint64_t)) {
            throw std::string("timerfd read() returned wrong num bytes");
        }
        return true;
    }

    // Only works if it was constructed with EFD_NONBLOCK
    bool tryRead() {
        uint64_t val = 0;
        return tryRead(val);
    }

    TimerFd & operator = (const TimerFd & other) = delete;
    TimerFd & operator = (TimerFd && other)
        noexcept
    {
        fd_ = other.fd_;
        other.fd_ = -1;

        return *this;
    }

    int fd_;
};

} // namespace common
#endif // _COMMON_EV_TIMER_FD_H_
