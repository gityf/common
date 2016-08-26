/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class Process.
*/

#pragma once
#include <memory>
#include <functional>
#include <set>
#include <string>
#include <unistd.h>
using std::string;

using ProcessCB_t = std::function<void()>;
class Process {
 public:
    // constructor.
    Process();

    // Destructor.
    ~Process();

    // run shell command.
    string RunShell(const string& aShell, const string& aResFile,
        int aTimeout = 5);
    // parent wait and all child.
    void wait(size_t num, const ProcessCB_t& callback);
    // stop all the child.
    void stop();
    // is this main process.
    bool isMainProc() {
        return mMainPid == mFd;
    }
    // child exist or not.
    bool hasChild() {
        return mChildren.size() > 0;
    }
    // child size.
    size_t childSize() {
        return mChildren.size();
    }

private:
    // create process.
    pid_t create();
    // check process is stop or not.
    void checkStop();

private:
    pid_t mMainPid;
    bool mRunning;
    std::set<pid_t> mChildren;

    int mFd;
};
