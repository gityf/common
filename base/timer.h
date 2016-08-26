// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/03/2015
// Description: file of timer.


#ifndef _COMMON_CTIMER_H_
#define _COMMON_CTIMER_H_
#include <stdio.h>
///////////////////////////////////////////////////////////////////////////////
// class Timer
////////////////////////////////////////////////////////////////////////////////
class Timer
{
public:
    class CallBackIf {
    public:
        virtual ~CallBackIf() {}
        virtual void OnTimer(Timer* pTimer, unsigned int id, void* ptr) = 0;
    };
    Timer();

    ~Timer();

    unsigned int schedule(Timer::CallBackIf* callback, unsigned int interval_ms,
                          int repeat = 1, void* ptr = NULL);

    unsigned int cancel(unsigned int id);

    long next_timed();

    bool start();

    bool stop();
private:
    class TimerImpl;
    TimerImpl* m_impl;
};

#endif//_COMMON_CTIMER_H_
