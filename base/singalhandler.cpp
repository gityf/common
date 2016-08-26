// Copyright (C) 2015 Wang Yaofu
// All rights reserved.
//
// Author:Wang Yaofu voipman@qq.com
// Description: The source file of class Signal.
//

#include <map>
#include <signal.h>
#include "singalhandler.h"

namespace {
    std::map<int, SignalFuncPtr> handlers;
    void signal_handler(int sig) {
        handlers[sig]();
    }
}
void Signal::signal(int sig, const SignalFuncPtr& handler) {
    handlers[sig] = handler;
    ::signal(sig, signal_handler);
}
// end of local file.