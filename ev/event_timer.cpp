#include "event_timer.h"

/*
 * the event timer rbtree may contain the duplicate keys, however,
 * it should not be a problem, because we use the rbtree to find
 * a minimum timer value only
 */

ngx_rbtree_node_t  EventTimer::sentinel_;

int EventTimer::init()
{
    //std::lock_guard<std::mutex> lock(mtx_);
    
    ngx_rbtree_init(&rbtree_, &sentinel_, ngx_rbtree_insert_timer_value);
    updateTime();
    return 0;
}

unsigned EventTimer::findTimer()
{
    int  timer;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    //std::lock_guard<std::mutex> lock(mtx_);
    
    if (rbtree_.root == &sentinel_) {
        return CURL_TIMER_INFINITE;
    }

    root = rbtree_.root;
    sentinel = rbtree_.sentinel;

    node = ngx_rbtree_min(root, sentinel);

    timer = (int) (node->key - currentMsec_);

    return (unsigned) (timer > 0 ? timer : 0);
}

void EventTimer::expireTimers()
{
    TimerEvent        *ev;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    //std::unique_lock<std::mutex> lock(mtx_);
    
    sentinel = rbtree_.sentinel;

    for ( ;; ) {
        root = rbtree_.root;
        if (root == sentinel) {
            return;
        }

        node = ngx_rbtree_min(root, sentinel);

        /* node->key > ngx_current_time */
        if ((int) (node->key - currentMsec_) > 0) {
            return;
        }

        ev = (TimerEvent *) ((char *) node - offsetof(TimerEvent, timer));

        //ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
        //               "event timer del: %d: %M",
        //               ngx_event_ident(ev->data), ev->timer.key);

        ngx_rbtree_delete(&rbtree_, &ev->timer);

        ev->timer_set = 0;
        ev->timedout = 1;

        //lock.unlock();
        ev->handler(ev);
        //lock.lock();
    }
}

void EventTimer::cancelTimers()
{
    TimerEvent        *ev;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    //std::unique_lock<std::mutex> lock(mtx_);
    
    sentinel = rbtree_.sentinel;

    for ( ;; ) {
        root = rbtree_.root;
        if (root == sentinel) {
            return;
        }

        node = ngx_rbtree_min(root, sentinel);

        ev = (TimerEvent *) ((char *) node - offsetof(TimerEvent, timer));
        if (!ev->cancelable) {
            return;
        }

        //ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
        //               "event timer cancel: %d: %M",
        //               ngx_event_ident(ev->data), ev->timer.key);

        ngx_rbtree_delete(&rbtree_, &ev->timer);
        ev->timer_set = 0;

        //lock.unlock();
        ev->handler(ev);
        //lock.lock();
    }
}
