// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/04/2015
// Description: Simple class that provides an FD that we can use to wake
// something up.  A generalization of the signal.

#ifndef _COMMON_EV_SIGNAL_FD_H_
#define _COMMON_EV_SIGNAL_FD_H_

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>

namespace common {

struct SignalFd {
    SignalFd(int aSigNo, int aFlags = 0)
    {
        sigemptyset(&mask_);
        sigaddset(&mask_, aSigNo);
        // aFlags SFD_NONBLOCK、SFD_CLOEXEC
        fd_ = ::signalfd(-1, &mask_, aFlags);
        if (fd_ == -1) {
            throw std::string("signalfd");
        }
    }

    SignalFd(const SignalFd & other) = delete;
    SignalFd(SignalFd && other)
        noexcept
        : fd_(other.fd_)
    {
        other.fd_ = -1;
        memset(&mask_, 0, sizeof(sigset_t));
    }

    ~SignalFd() {
        if (fd_ != -1) {
            ::close(fd_);
        }
    }

    int fd() const { return fd_; }

    void signal(int aSigNo, int aFlags = 0) {
        sigset_t mask;
        sigemptyset(&mask);
        memcpy(&mask, &mask_, sizeof(sigset_t));
        sigaddset(&mask, aSigNo);
        memcpy(&mask_, &mask, sizeof(sigset_t));
        fd_ = ::signalfd(-1, &mask_, aFlags);
        if (fd_ == -1) {
            throw std::string("signalfd write()");
        }
    }

    // return signal no
    int read() {
        struct signalfd_siginfo fdsi;
        int res = ::read(fd_, &fdsi, sizeof(struct signalfd_siginfo));
        if (res != sizeof(struct signalfd_siginfo)) {
            throw std::string("signalfd read()");
        }
        // fdsi.ssi_signo maybe SIGINT SIGQUIT ...
        return fdsi.ssi_signo;
    }

    // Only works if it was constructed with EFD_NONBLOCK
    bool tryRead(int & val) {
        struct signalfd_siginfo fdsi;
        int res = ::read(fd_, &fdsi, sizeof(struct signalfd_siginfo));
        if (res == -1 && errno == EWOULDBLOCK) {
            return false;
        }
        if (res == -1) {
            throw std::string("signalfd read()");
        }
        if (res != sizeof(struct signalfd_siginfo)) {
            throw std::string("signalfd tryRead()");
        }
        return true;
    }

    // Only works if it was constructed with EFD_NONBLOCK
    bool tryRead()
    {
        int val = 0;
        return tryRead(val);
    }

    SignalFd & operator = (const SignalFd & other) = delete;
    SignalFd & operator = (SignalFd && other)
        noexcept
    {
        fd_ = other.fd_;
        other.fd_ = -1;
        memcpy(&mask_, &other.mask_, sizeof(sigset_t));
        memset(&other.mask_, 0, sizeof(sigset_t));
        return *this;
    }

    int fd_;
    sigset_t mask_;
};

} // namespace common
#endif /* _COMMON_EV_SIGNAL_FD_H_ */
