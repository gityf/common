// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/03/2015
// Description: file of timer.

#include <map>
#include <string>
#include "timer.h"
#include "thread.h"
#include "atomic/atomic.h"
#include "ipc/semaphore.h"

static int Timer_ID_Generater(0);

///////////////////////////////////////////////////////////////////////////////
// class TimerTask
////////////////////////////////////////////////////////////////////////////////
class TimerTask
{
public:
    unsigned int       mId;
    // unit: ms
    unsigned int       mIntervalMs;
    // less than 0:repeat forever, 0: stop, big than 0: left repeat times.
    int                mRepeats;
    long               mExpires;
    volatile bool      mDelFlag;
    void*              mPtrData;
    Timer::CallBackIf*  mCallBack;

    TimerTask(Timer::CallBackIf* callback, unsigned int interval_ms, int repeat, void* ptr = NULL)
        : mIntervalMs(interval_ms)
        , mRepeats(repeat)
        , mPtrData(ptr)
        , mDelFlag(false)
        , mCallBack(callback) {
        mId = AtomicIncrement(&Timer_ID_Generater);
        update();
    }

    ~TimerTask() {
        if (mCallBack) {
            delete mCallBack;
        }
    }

    void update() {
        mExpires = ThreadTimer::instance()->getNowMSecs() +  mIntervalMs;
    }
};

///////////////////////////////////////////////////////////////////////////////
// class Timer::TimerImpl
////////////////////////////////////////////////////////////////////////////////
class Timer::TimerImpl : public Thread
{
public:
    TimerImpl(){}

    TimerImpl(Timer* pTimer);

    virtual ~TimerImpl() {
        stop();
    }

    unsigned int schedule(Timer::CallBackIf* callback,
                          unsigned int interval_ms,
                          int repeat = 1, void* ptr = NULL);

    unsigned int cancel(unsigned int id);

    unsigned int cancel(void* ptr);

    long  next_timed();
private:
    /*
    * start the thread.
    *
    */
    void run();

    /*
    * stop the thread.
    *
    */
    void on_stop();

    bool _once_run();

private:
    typedef std::multimap<long, TimerTask*> Table;
    Table  mTasks;
    Mutex  mMutex;
    Timer* mpTimer;
    bool   mIsStoped;
    common::Semaphore mSemaphore;
};

Timer::TimerImpl::TimerImpl(Timer* pTimer)
    : mpTimer(pTimer) {
    mIsStoped = false;
}

void Timer::TimerImpl::on_stop() {
    mIsStoped = true;
    ScopeLock lock(mMutex);
    for (auto it = mTasks.begin(); it != mTasks.end(); ++it) {
        if (it->second != NULL) delete it->second;
    }
    mTasks.clear();
    size_t size = mTasks.size();
    while (size-- > 0) {
        mSemaphore.Post();
    }
    mSemaphore.Post();
}

unsigned int Timer::TimerImpl::schedule(Timer::CallBackIf* callback,
    unsigned int interval_ms, int repeat, void* ptr) {
    if (repeat > 1 && interval_ms == 0) interval_ms = 1;
    TimerTask* task_ = new TimerTask(callback, interval_ms, repeat, ptr);
    ScopeLock lock(mMutex);  // auto lock
    mTasks.insert(std::make_pair(task_->mExpires, task_));
    mSemaphore.Post();
    return task_->mId;
}

unsigned int Timer::TimerImpl::cancel(unsigned int id) {
    unsigned int count = 0;
    if (id == 0) return 0;

    ScopeLock lock(mMutex);
    for (auto it = mTasks.begin(); it != mTasks.end(); ++it) {
        TimerTask* task_ = it->second;
        if (task_->mId == id) {
            if (!task_->mDelFlag) {
                task_->mDelFlag = true;
                count++;
            }
            break;
        }
    }

    mSemaphore.Post();
    return count;
}

long Timer::TimerImpl::next_timed() {
    ScopeLock lock(mMutex);
    if (mTasks.empty()) return -1;
    long now = ThreadTimer::instance()->getNowMSecs();
    TimerTask* task_ = mTasks.begin()->second;
    if (task_->mExpires <= now) return 0;
    return task_->mExpires - now;
}

void Timer::TimerImpl::run() {
    while (!mIsStoped) {
        _once_run();
    }
}

bool Timer::TimerImpl::_once_run() {
    TimerTask* task_ = NULL;
    bool bRet = false;
    long now = ThreadTimer::instance()->getNowMSecs();

    mMutex.lock();
    long time_to_wait = 0;
    bool isempty = false;
    if (!mTasks.empty()) {
        task_ = mTasks.begin()->second;
        if (!task_) {
        }
        else if (task_->mDelFlag) {
            mTasks.erase(mTasks.begin());
            delete task_;
            task_ = NULL;
            bRet = true;
        }
        else if (task_->mRepeats == 0) {
            mTasks.erase(mTasks.begin());
            delete task_;
            task_ = NULL;
            bRet = true;
        }
        else if (task_->mExpires > now) {
            time_to_wait = task_->mExpires - now;
            task_ = NULL;
            bRet = false;
        }
        else {
            if (task_->mRepeats > 0) {
                task_->mRepeats--;
                mTasks.erase(mTasks.begin());
                task_->mExpires = now + task_->mIntervalMs;
                mTasks.insert(std::make_pair(task_->mExpires, task_));
                bRet = true;
            }
        }
    }
    else {
        isempty = true;
    }
    mMutex.unlock();

    if (isempty) {
        mSemaphore.Wait();
        return bRet;
    }

    if (time_to_wait > 0) {
        mSemaphore.TryWait((unsigned int)time_to_wait);
        return bRet;
    }

    if (task_ == NULL) return bRet;

    task_->mCallBack->OnTimer(mpTimer, task_->mId, task_->mPtrData);

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
// class Timer
////////////////////////////////////////////////////////////////////////////////
Timer::Timer()
    : m_impl(new TimerImpl(this)) {
}

Timer::~Timer() {
    delete m_impl;
}

unsigned int Timer::schedule(Timer::CallBackIf* callback,
    unsigned int interval_ms, int repeat, void* ptr) {
    return m_impl->schedule(callback, interval_ms, repeat, ptr);
}

unsigned int Timer::cancel(unsigned int id) {
    return m_impl->cancel(id);
}

bool Timer::start() {
    try {
        m_impl->start();
    } catch(const std::string& e) {
        return false;
    }
    return true;
}

bool Timer::stop() {
    m_impl->stop();
    return true;
}

long Timer::next_timed() {
    return m_impl->next_timed();
}
