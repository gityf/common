// Copyright (C) 2015 Wang Yaofu
// All rights reserved.
//
// Author:Wang Yaofu voipman@qq.com
// Description: The source file of class SetTimer.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "setitimer.h"

namespace {
    TimerFuncPtr handler_;
    void alarm_handler(int sig) {
        if (handler_ != NULL) handler_();
    }
}

bool SetTimer::set(long intervalMs, TimerFuncPtr handler) {
    handler_ = handler;
    // ::signal(SIGALRM, alarm_handler);
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = alarm_handler;
    ::sigemptyset(&sa.sa_mask);

    if (::sigaction(SIGALRM, &sa, NULL) == -1) {
        return false;
    }

    struct itimerval itv;
    memset(&itv, 0, sizeof(struct itimerval));
    if (intervalMs > 0) {
        itv.it_interval.tv_sec = intervalMs / 1000;
        itv.it_interval.tv_usec = (intervalMs % 1000) * 1000;
        itv.it_value.tv_sec = intervalMs / 1000;
        itv.it_value.tv_usec = (intervalMs % 1000 ) * 1000;
    }
    if (::setitimer(ITIMER_REAL, &itv, NULL) == -1) {
        return true;
    }
    return true;
}

void SetTimer::clear() {
    handler_ = NULL;
    struct itimerval itv;
    memset(&itv, 0, sizeof(struct itimerval));
    ::setitimer(ITIMER_REAL, &itv, NULL);
}

// This is a example.
// #include <iostream>
// #include <unistd.h>
// #include "setitimer.h"
// 
// using namespace std;
// 
// void timer() {
//     cout << "timer called at" << time(NULL) << endl;
// }
// 
// int main() {
//     SetTimer setTimer;
//     setTimer.set(1000, timer);
//     while(true) {
//        sleep(1);
//     }
//     return 0;
// }

// end of local file.
