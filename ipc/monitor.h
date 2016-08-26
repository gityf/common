#ifndef _COMMON_IPC_MONITOR_H__
#define _COMMON_IPC_MONITOR_H__
#include "semaphore.h"

class Monitor {
public:
    Monitor() {}

    ~Monitor() {}

    void TimeWait(unsigned int aMsec) {
        mSem.TryWait(aMsec);
    }

    void Wait() {
         mSem.Wait();
    }

    void Notify() {
        mSem.Post();
    }

private:
    Semaphore mSem;
};

#endif // _COMMON_IPC_MONITOR_H__
