#ifndef _NGX_EVENT_TIMER_H_INCLUDED_
#define _NGX_EVENT_TIMER_H_INCLUDED_

#include "ngx_rbtree.h"

#include <sys/time.h>

#include <cstddef>
#include <cstdlib>
#include <functional>
//#include <mutex>

#define CURL_TIMER_INFINITE  (ngx_msec_t) -1
#define CURL_TIMER_LAZY_DELAY  300

typedef ngx_rbtree_key_t      ngx_msec_t;

struct TimerEvent;
typedef std::function<void (TimerEvent*)> TimerHandler;

struct TimerEvent {
    bool cancelable;
    bool timedout;
    bool timer_set;
    ngx_rbtree_node_t timer;
    TimerHandler handler;
};

class EventTimer {
  public:
    int init();
    unsigned findTimer();
    void expireTimers();
    void cancelTimers();

  public:
    void updateTime() {
        time_t           sec;
        ngx_uint_t       msec;
        struct timeval   tv;

        gettimeofday(&tv, NULL);

        sec = tv.tv_sec;
        msec = tv.tv_usec / 1000;
        currentMsec_ = (unsigned) sec * 1000 + msec;
    }

    unsigned currentMillis() {
        return currentMsec_;
    }

    void delTimer(TimerEvent *ev) {
        //ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
        //               "event timer del: %d: %M",
        //                ngx_event_ident(ev->data), ev->timer.key);

        //std::lock_guard<std::mutex> lock(mtx_);
        ngx_rbtree_delete(&rbtree_, &ev->timer);
        ev->timer_set = 0;
    }

    void addTimer(TimerEvent *ev, unsigned timer) {
        unsigned      key;
        ngx_msec_int_t  diff;

        key = currentMsec_ + timer;

        if (ev->timer_set) {
            /*
             * Use a previous timer value if difference between it and a new
             * value is less than NGX_TIMER_LAZY_DELAY milliseconds: this allows
             * to minimize the rbtree operations for fast connections.
             */
            diff = (ngx_msec_int_t) (key - ev->timer.key);
            if (abs(diff) < CURL_TIMER_LAZY_DELAY) {
                //ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                //               "event timer: %d, old: %M, new: %M",
                //                ngx_event_ident(ev->data), ev->timer.key, key);
                return;
            }

            delTimer(ev);
        }

        ev->timer.key = key;

        //ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
        //               "event timer add: %d: %M:%M",
        //                ngx_event_ident(ev->data), timer, ev->timer.key);

        //std::lock_guard<std::mutex> lock(mtx_);
        ngx_rbtree_insert(&rbtree_, &ev->timer);
        ev->timer_set = 1;
    }

  private:
    //std::mutex mtx_;
    ngx_rbtree_t  rbtree_;
    static ngx_rbtree_node_t  sentinel_;
    unsigned currentMsec_;
};

#endif /* _NGX_EVENT_TIMER_H_INCLUDED_ */
