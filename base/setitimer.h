// Copyright (C) 2015 Wang Yaofu
// All rights reserved.
//
// Author:Wang Yaofu voipman@qq.com
// Description: The header file of class SetTimer.
//

#pragma once

typedef void (*TimerFuncPtr)();

struct SetTimer {
    // true for success, false for error.
    bool set(long intervalMs, TimerFuncPtr handler);
    void clear();
};