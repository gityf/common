/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class Process.
*/

#include <stdio.h>
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/signalfd.h>
#include "process.h"
#include "log.h"
#include "foreach.h"

using namespace std::placeholders;

Process::Process() {
}

Process::~Process() {
}

static void ProcessSigAlarm(int aSinid) {
    return;
}

string Process::RunShell(const string& aShell,
    const string& aResFile, int aTimeout) {
    DEBUG(LL_ALL, "Begin.");
    LOG(LL_VARS, "shell cmd=(%s)", aShell.c_str());

    FILE *pFile = ::popen(aShell.c_str(), "r");
    string resultStr = "";
    bool isSysCall = false;
    if (pFile != NULL) {
        LOG(LL_NOTICE, "in mode 1.");
    } else {
        resultStr = aShell;
        resultStr += " > ";
        resultStr += aResFile;
        ::system(resultStr.c_str());
        pFile = ::fopen(aResFile.c_str(), "r");
        if (pFile != NULL) {
            isSysCall = true;
            LOG(LL_NOTICE, "in mode 2.");
        }
    }
    resultStr = "";
    if (pFile != NULL) {
        char resbuf[1024] = {0};
        ::signal(SIGALRM, ProcessSigAlarm);
        ::alarm(aTimeout);
        while(fgets(resbuf, sizeof(resbuf), pFile) != NULL) {
            resultStr += resbuf;
        }
        ::alarm(0);
        if (isSysCall) {
            ::fclose(pFile);
            ::unlink(aResFile.c_str());
            ::remove(aResFile.c_str());
        } else {
            ::pclose(pFile);
        }
    }    
    DEBUG(LL_ALL, "End.");
    return resultStr;
}

pid_t Process::create() {
    pid_t pid = ::fork();
    if (pid < 0) {
        LOG_ERROR("fork error.");
        return 0;
    } else if (pid == 0) {
        //child
        mChildren.clear();
        ::close(mFd);
        return ::getpid();
    } else {
        //parent 
        mChildren.insert(pid);
    }

    return 0;
}

void Process::wait(size_t num, const ProcessCB_t& callback) {
    mRunning = true;
    for (size_t i = 0; i < num; ++i) {
        pid_t pid = create();
        if (pid != 0) {
            // child process do the callback.
            callback();
            return;
        }
    }

    while (!mChildren.empty()) {
        int status = 0;
        pid_t pid;
        if ((pid = ::waitpid(-1, &status, WNOHANG)) > 0) {
            mChildren.erase(pid);

            if (!mRunning) {
                continue;
            }

            LOG_INFO("child was dead, restart it");

            if (create() != 0) {
                callback();
                return;
            }
        }
        else {
            checkStop();
            ::sleep(1);
            continue;
        }
    }

    return;
}

void Process::stop() {
    LOG_INFO("stop child process");
    mRunning = false;
    common::for_each_all(mChildren,
        std::bind(::kill, _1, SIGTERM));
}

void Process::checkStop() {
    struct signalfd_siginfo fdsi;
    int s = ::read(mFd, &fdsi, sizeof(fdsi));

    if (s != sizeof(fdsi)) {
        //no signal, 
        return;
    }

    int signum = fdsi.ssi_signo;
    switch (signum) {
    case SIGTERM:
        stop();
        break;
    default:
        LOG_INFO("signum %d", signum);
        break;
    }
}