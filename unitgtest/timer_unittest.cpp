/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of Thread ut.
*/
#include <iostream>
#include <string>
#include "base/timer.h"
#include "ut/test_harness.h"
using namespace std;

class CleanerTimer : public Timer::CallBackIf {
    void OnTimer(Timer* pTimer, unsigned int id, void* ptr) {
        cout << "CleanerTimer::OnTimer:" << id << endl;
    }
};

TEST(CTimerTest, WithMethodTest)
{
    Timer timer;
    CleanerTimer *t1 = new CleanerTimer();
    CleanerTimer *t2 = new CleanerTimer();
    // t1 t2 will delete auto in timer.
    timer.schedule(t1, 100);
    timer.schedule(t2, 1000, 10);
    timer.start();
    getchar();
}

