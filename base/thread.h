/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class Thread,Mutex and ThreadWatcher.
*/

#ifndef _COMMON_THREAD_THREAD_H_
#define _COMMON_THREAD_THREAD_H_

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <queue>
/**
* \brief C++ Wrapper class for pthread mutex
*/
class Mutex {
    pthread_mutex_t m;

 public:
    Mutex() {
        pthread_mutex_init(&m,NULL);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m);
    }
    void lock() {
        pthread_mutex_lock(&m);
    }

    void unlock() {
        pthread_mutex_unlock(&m);
    }
};

/**
* \brief  Simple lock class
*/
class ScopeLock {
    Mutex& m;
 public:
    explicit ScopeLock(Mutex& _m) : m(_m) {
        m.lock();
    }
    ~ScopeLock(){
        m.unlock();
    }
};

class SpinLock {
 public:
    SpinLock() {
        pthread_spin_init(&mSpin, PTHREAD_PROCESS_PRIVATE);
    }
    ~SpinLock() {
        pthread_spin_destroy(&mSpin);
    }
    void Lock() {
        pthread_spin_lock(&mSpin);
    }
    void Unlock() {
        pthread_spin_unlock(&mSpin);
    }
    bool IsLocked() const {
        return mSpin == 0;
    }

 private:
    pthread_spinlock_t mSpin;
};

/**
* \brief  Simple ScopedSpinLock class
*/
class ScopedSpinLock {
    SpinLock& m;
 public:
    explicit ScopedSpinLock(SpinLock& _m) : m(_m) {
        m.Lock();
    }
    ~ScopedSpinLock(){
        m.Unlock();
    }
};

//
// Read-Write Lock
//
class RWLock {
 public:
    // constructor
    RWLock() {
        // default lock attr
        pthread_rwlock_init(&m_rwLock, NULL);
    }

    // destructor
    virtual ~RWLock() {
        pthread_rwlock_destroy(&m_rwLock);
    }

 public:
    // lock read lock
    bool useRD() {
        return (pthread_rwlock_rdlock(&m_rwLock) == 0) ? true : false;
    }

    // try to lock read lock
    bool tryRD() {
        return (pthread_rwlock_tryrdlock(&m_rwLock) == 0) ? true : false;
    }

    // lock write lock
    bool useWR() {
        return (pthread_rwlock_wrlock(&m_rwLock) == 0) ? true : false;
    }

    // lock write lock
    bool tryWR() {
        return (pthread_rwlock_trywrlock(&m_rwLock) == 0) ? true : false;
    }

    // unlock read-write lock
    bool unlock() {
        return (pthread_rwlock_unlock(&m_rwLock) == 0) ? true : false;
    }

 private:
    // read-write lock
    pthread_rwlock_t m_rwLock;
};

//
// Read Scope lock
//
class ScopeRDLock {
 public:
    // constructor
    explicit ScopeRDLock(RWLock &rdlock) : m_srdLock(rdlock) {
        m_srdLock.useRD();
    }

    // destructor
    virtual ~ScopeRDLock() {
        m_srdLock.unlock();
    }

 private:
    // read lock
    RWLock &m_srdLock;
};

//
// write scope lock
//
class ScopeWRLock {
 public:
    // constructor
    explicit ScopeWRLock(RWLock &wrlock) : m_swrLock(wrlock) {
        m_swrLock.useWR();
    }

    // destructor
    virtual ~ScopeWRLock() {
        m_swrLock.unlock();
    }

 private:
    // read lock
    RWLock &m_swrLock;
};
/**
* \brief Shared variable.
*
* Include a variable and its mutex.
* @warning Don't use safe functions (set,get)
* within a {lock(); ... unlock();} block. Use
* unsafe function instead.
*/
template<class T>
class SharedVar {
    T t;
    Mutex m;

 public:
    explicit SharedVar(const T& _t) : t(_t) {}
    SharedVar() {}

    T get() {
        lock();
        T res = unsafe_get();
        unlock();
        return res;
    }

    void set(const T& new_val) {
        lock();
        unsafe_set(new_val);
        unlock();
    }

    void lock() { m.lock(); }
    void unlock() { m.unlock(); }

    const T& unsafe_get() { return t; }
    void unsafe_set(const T& new_val) { t = new_val; }
};

/**
* \brief C++ Wrapper class for pthread condition
*/
template<class T>
class Condition {
    T t;
    pthread_mutex_t m;
    pthread_cond_t  cond;

 public:
    explicit Condition(const T& _t)
        : t(_t) {
        pthread_mutex_init(&m, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~Condition() {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&m);
    }

    /** Change the condition's value. */
    void set(const T& newval) {
        pthread_mutex_lock(&m);
        t = newval;
        if (t) pthread_cond_signal(&cond);
        pthread_mutex_unlock(&m);
    }

    T get() {
        T val;
        pthread_mutex_lock(&m);
        val = t;
        pthread_mutex_unlock(&m);
        return val;
    }
    /** Waits for the condition to be true. */
    void wait_for() {
        pthread_mutex_lock(&m);
        while (!t) {
            pthread_cond_wait(&cond, &m);
        }
        pthread_mutex_unlock(&m);
    }

    /** Waits for the condition to be true or a timeout. */
    bool wait_for_to(unsigned long usec) {
        struct timeval now;
        struct timespec timeout;
        int retcode = 0;
        bool ret = false;

        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec + (usec / 1000000);
        timeout.tv_nsec = (now.tv_usec + (usec % 1000000)) * 1000;

        pthread_mutex_lock(&m);
        while (!t && !retcode) {
            retcode = pthread_cond_timedwait(&cond, &m, &timeout);
        }

        if (t) ret = true;
        pthread_mutex_unlock(&m);

        return ret;
    }
};

template<class T>
class RWCondition {
    T r_t;
    T w_t;
    pthread_mutex_t m;
    pthread_cond_t  r_cond;
    pthread_cond_t  w_cond;

 public:
    explicit RWCondition(const T& _t)
        : r_t(_t), w_t(_t) {
        pthread_mutex_init(&m, NULL);
        pthread_cond_init(&r_cond, NULL);
        pthread_cond_init(&w_cond, NULL);
    }

    ~RWCondition() {
        pthread_cond_destroy(&r_cond);
        pthread_cond_destroy(&w_cond);
        pthread_mutex_destroy(&m);
    }

    /** Change the condition's value. */
    void set(const T& newval, bool isR) {
        if (isR) {
            r_t = newval;
            if (r_t) pthread_cond_signal(&r_cond);
        } else {
            w_t = newval;
            if (w_t) pthread_cond_signal(&w_cond);
        }
    }
    void lock() {
        pthread_mutex_lock(&m);
    }
    void unlock() {
        pthread_mutex_unlock(&m);
    }
    void wait_for(bool isR) {
        if (isR) {
            while (!r_t) {
                pthread_cond_wait(&r_cond, &m);
            }
        } else {
            while (!w_t) {
                pthread_cond_wait(&w_cond, &m);
            }
        }
    }
};

/**
* \brief C++ Wrapper class for pthread
*/
class Thread {
    pthread_t _td;
    Mutex   _m_td;

    SharedVar<bool> _stopped;

    static void* _start(void* _t);

 protected:
    virtual void run() {}
    virtual void on_stop() {}

 public:
    unsigned long _pid;
    Thread();
    virtual ~Thread() {}

    virtual void onIdle() {}

    /** Start it ! */
    void start(bool realtime = false);
    /** Stop it ! */
    void stop();
    /** @return true if this thread doesn't run. */
    bool is_stopped() { return _stopped.get(); }
    /** Wait for this thread to finish */
    void join();
    /** kill the thread
     *(if pthread_setcancelstate(PTHREAD_CANCEL_ENABLED) has been set) **/
    void cancel();

    int setRealtime();
};

/**
* \brief Container/garbage collector for threads.
*
* ThreadWatcher waits for threads to stop
* and delete them.
* It gets started automatically when needed.
* Once you added a thread to the container,
* there is no mean to get it out.
*/
class ThreadWatcher: public Thread {
    static ThreadWatcher* _instance;
    static Mutex          _inst_mut;
    /*
    * the condition of thread to start or stop.
    *
    */
    SharedVar<bool> mStopRequested;

    std::queue<Thread*> thread_queue;
    Mutex          q_mut;

    /** the daemon only runs if this is true */
    Condition<bool> _run_cond;
    ThreadWatcher();
    void run();
    void on_stop();

 public:
    static ThreadWatcher* instance();
    void add(Thread* t);
    /*
    * delete instance
    *
    */
    static void Destroy();
};

class ThreadTimer: public Thread {
    static ThreadTimer* _instance;
    /*
    * the condition of thread to start or stop.
    *
    */
    bool mStopRequested;

    /** the daemon only runs if this is true */
    bool _run_cond;
    ThreadTimer();
    void run();
    void on_stop();
    struct timeval mTimeVal;

 public:
    static ThreadTimer* instance();
    time_t getNowSecs() const {
        return mTimeVal.tv_sec;
    }
    long getNowMSecs() {
        gettimeofday(&mTimeVal, NULL);
        return (mTimeVal.tv_sec) * 1000 + mTimeVal.tv_usec / 1000;
    }
    void getTimeOfDay(struct timeval* aOutTV) {
        gettimeofday(&mTimeVal, NULL);
        aOutTV->tv_sec = mTimeVal.tv_sec;
        aOutTV->tv_usec = mTimeVal.tv_usec;
    }

    // get the difference of two time structure 
    long getDiffTvInUSec(struct timeval& tvStop, struct timeval &tvStart) {
        return tvStop.tv_usec - tvStart.tv_usec + 1000000 * (tvStop.tv_sec - tvStart.tv_sec);
    }
    // get the difference of two time structure 
    long getDiffTvInMSec(struct timeval& tvStop, struct timeval &tvStart) {
        return (tvStop.tv_usec - tvStart.tv_usec) / 1000 + 1000 * (tvStop.tv_sec - tvStart.tv_sec);
    }

    /*
    * delete instance
    *
    */
    static void Destroy();
};
#endif  // _COMMON_THREAD_THREAD_H_
