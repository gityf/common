// Copyright (C) 2015 Wang Yaofu
// All rights reserved.
//
// Author:Wang Yaofu voipman@qq.com
// Description: The header file of class Signal.
//

#pragma once
#include <functional>

using SignalFuncPtr = std::function<void()>;

struct Signal {
    void signal(int sig, const SignalFuncPtr& handler);
};