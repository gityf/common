/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Source file of class TinyThread.
*/

#include <assert.h>
#include <stdlib.h>
#include <string>
#include "uncopyable.h"
#include "tinythread.h"

namespace {
void* ThreadFunc(void* ctx)
{
    Closure<void>* func = reinterpret_cast<Closure<void>*>(ctx);
    func->Run();
    return NULL;
}

} // anonymous namespace

class TinyThread : private Uncopyable
{
private:
    typedef pthread_t handle_type;
    TinyThread(Closure<void>* func, const char* name);

    ~TinyThread();

    void Routine();

private:
    friend pthread_t CreateThread(Closure<void>* func, const char *aName);

    std::string mName;
    Closure<void>* mFunc;
    handle_type mHandle;
};

TinyThread::TinyThread(Closure<void>* func, const char* name)
  : mFunc(func),
    mHandle(0)
{
    if (name != NULL) {
        mName = name;
    }
    Closure<void>* routine = NewClosure(this, &TinyThread::Routine);
    if (::pthread_create(
            &mHandle, NULL, ThreadFunc, reinterpret_cast<void*>(routine))) {
        ::abort();
    }
}

TinyThread::~TinyThread()
{
}

void TinyThread::Routine()
{
    mFunc->Run();
    delete this;
}

ThreadHandle CreateThread(Closure<void>* func, const char* name)
{
    TinyThread* thread = new TinyThread(func, name);
    assert(thread);
    TinyThread::handle_type handle = thread->mHandle;
    ::pthread_detach(handle);
    return handle;
}

