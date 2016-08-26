/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class Thread,Mutex and ThreadWatcher.
*/

#include <unistd.h>
#include <cerrno>
#include <string>
#include "thread.h"
#include "log.h"
using std::string;

Thread::Thread()
: _stopped(true) , _pid(0) {
}

// int thread_nr=0;
// Mutex thread_nr_mut;

void * Thread::_start(void * _t) {
    Thread* _this = (Thread*)_t;
    _this->_pid = (unsigned long) _this->_td;
    LOG(LL_INFO, "Thread::Thread %lu is starting.",
        (unsigned long) _this->_pid);
    _this->_stopped.set(false);
    _this->run();

    LOG(LL_INFO, "Thread::Thread %lu is ending.",
        (unsigned long) _this->_pid);
    _this->_stopped.set(true);

    //thread_nr_mut.lock();
    //INFO("threads = %i\n",--thread_nr);
    //thread_nr_mut.unlock();

    return NULL;
}

void Thread::start(bool realtime) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr,1024*1024*2);// 1 MB
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    int res;
    _pid = 0;
    // unless placed here, a call seq like run(); join(); will not wait to join
    // b/c creating the thread can take too long
    this->_stopped.set(false);
    res = pthread_create(&_td,&attr,_start,this);
    pthread_attr_destroy(&attr);
    if (res != 0) {
        LOG(LL_ERROR, "Thread::start():pthread create failed with code %i", res);
        throw string("Thread::start():thread could not be started");
    }
}

void Thread::stop() {
    _m_td.lock();

    if(is_stopped()){
        _m_td.unlock();
        return;
    }

    // gives the thread a chance to clean up
    LOG(LL_INFO, "Thread::stop():Thread %lu (%lu) calling on_stop, give it a chance to clean up.",
        (unsigned long int) _pid, (unsigned long int) _td);

    try { on_stop(); } catch(...) {}

    int res;
    if ((res = pthread_detach(_td)) != 0) {
        if (res == EINVAL) {
            LOG(LL_ERROR, "Thread::stop():pthread_detach failed with code EINVAL: thread already in detached state.");
        } else if (res == ESRCH) {
            LOG(LL_ERROR, "Thread::stop():pthread_detach failed with code ESRCH: thread could not be found.");
        } else {
            LOG(LL_ERROR, "Thread::stop():pthread_detach failed with code %i", res);
        }
    }

    LOG(LL_INFO, "Thread::stop():Thread %lu (%lu) finished detach.",
        (unsigned long int) _pid, (unsigned long int) _td);

    //pthread_cancel(_td);

    _m_td.unlock();
}

void Thread::cancel() {
    _m_td.lock();

    int res;
    if ((res = pthread_cancel(_td)) != 0) {
        LOG(LL_INFO, "Thread::cancel():pthread_cancel failed with code %i", res);
    } else {
        LOG(LL_INFO, "Thread::cancel():Thread %lu is canceled.", (unsigned long int) _pid);
        _stopped.set(true);
    }

    _m_td.unlock();
}

void Thread::join() {
    if(!is_stopped())
        pthread_join(_td,NULL);
}

int Thread::setRealtime() {
    return 0;
}


ThreadWatcher* ThreadWatcher::_instance=0;
Mutex ThreadWatcher::_inst_mut;

ThreadWatcher::ThreadWatcher()
: _run_cond(false),mStopRequested(false) {
}

ThreadWatcher* ThreadWatcher::instance() {
    _inst_mut.lock();
    if(!_instance){
        _instance = new ThreadWatcher();
        _instance->start();
    }

    _inst_mut.unlock();
    return _instance;
}

void ThreadWatcher::add(Thread* t) {
    LOG(LL_INFO, "ThreadWatcher::add():trying to add thread %lu to thread watcher.",
        (unsigned long int) t->_pid);
    q_mut.lock();
    thread_queue.push(t);
    _run_cond.set(true);
    q_mut.unlock();
    LOG(LL_INFO, "ThreadWatcher::add():added thread %lu to thread watcher.",
        (unsigned long int) t->_pid);
}

// ---------------------------------------------------------------------------
// void ThreadWatcher::Destroy()
//
// delete instance.
// ---------------------------------------------------------------------------
//
void ThreadWatcher::Destroy() {
    DEBUG(LL_ALL, "ThreadWatcher::Destroy():Begin");
    if(_instance != NULL) {
        if(!_instance->is_stopped()) {
            _instance->stop();

            while(!_instance->is_stopped()) {
                usleep(10000);
            }
        }
        delete _instance;
        _instance = NULL;
    }
    DEBUG(LL_ALL, "ThreadWatcher::Destroy():End");
}

void ThreadWatcher::on_stop() {
    mStopRequested.set(true);
}

void ThreadWatcher::run() {
    while(!mStopRequested.get()){

        _run_cond.wait_for();
        // Let some time for to threads
        // to stop by themselves
        sleep(10);

        q_mut.lock();
        LOG(LL_VARS, "ThreadWatcher::run():Thread watcher starting its work");

        try {
            std::queue<Thread*> n_thread_queue;

            while(!thread_queue.empty()){

                Thread* cur_thread = thread_queue.front();
                thread_queue.pop();

                q_mut.unlock();
                LOG(LL_VARS, "ThreadWatcher::run():thread %lu is to be processed in thread watcher.",
                    (unsigned long int) cur_thread->_pid);
                if(cur_thread->is_stopped()){
                    LOG(LL_INFO, "ThreadWatcher::run():thread %lu has been destroyed.",
                        (unsigned long int) cur_thread->_pid);
                    delete cur_thread;
                }
                else {
                    LOG(LL_VARS, "ThreadWatcher::run():thread %lu still running.",
                        (unsigned long int) cur_thread->_pid);
                    n_thread_queue.push(cur_thread);
                }
                q_mut.lock();
            }

            swap(thread_queue,n_thread_queue);

        }catch(...){
            /* this one is IMHO very important, as lock is called in try block! */
            LOG(LL_ERROR, "ThreadWatcher::run():unexpected exception, state may be invalid!");
        }

        bool more = !thread_queue.empty();
        q_mut.unlock();

        LOG(LL_VARS, "ThreadWatcher::run():Thread watcher finished");
        if(!more)
            _run_cond.set(false);
    }
}

// timer...
ThreadTimer* ThreadTimer::_instance=0;

ThreadTimer::ThreadTimer()
: _run_cond(false),mStopRequested(false) {
    gettimeofday(&mTimeVal, NULL);
}

ThreadTimer* ThreadTimer::instance() {
    if(!_instance){
        _instance = new ThreadTimer();
    }
    return _instance;
}

// ---------------------------------------------------------------------------
// void ThreadWatcher::Destroy()
//
// delete instance.
// ---------------------------------------------------------------------------
//
void ThreadTimer::Destroy() {
    DEBUG(LL_ALL, "ThreadTimer::Destroy():Begin");
    if(_instance != NULL) {
        if(!_instance->is_stopped()) {
            _instance->stop();

            while(!_instance->is_stopped()) {
                usleep(10000);
            }
        }
        delete _instance;
        _instance = NULL;
    }
    DEBUG(LL_ALL, "ThreadTimer::Destroy():End");
}

void ThreadTimer::on_stop() {
    mStopRequested = true;
}

void ThreadTimer::run() {
    while(!mStopRequested){
        gettimeofday(&mTimeVal, NULL);
        // Let some time for to threads
        // to sleep
        usleep(800000);
    }
}
// end of local file.
