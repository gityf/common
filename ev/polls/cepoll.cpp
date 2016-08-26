/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CEventPoll by api epoll.
*/
#include <sys/epoll.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "base/log.h"
typedef struct SEventState {
    int epfd;
    struct epoll_event *events;
} SEventState;

int CEventPoll::ApiCreate(EventLoop * aEventLoop) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_INFO, "try malloc event state.");
    mEventState = reinterpret_cast<SEventState *>(malloc(sizeof(SEventState)));
    if (mEventState == NULL) {
        LOG(LL_ERROR, "malloc event state error:(%s).",
            strerror(errno));
        return RET_ERROR;
    }
    mEventLoop = aEventLoop;
    // 1024 is just an hint for the kernel
    mEventState->epfd = epoll_create(1024);
    if (mEventState->epfd == RET_ERROR) {
        LOG(LL_ERROR, "epoll_create error:(%s).",
            strerror(errno));
        return RET_ERROR;
    }
    mEventState->events =
        (struct epoll_event *)malloc(
        sizeof(struct epoll_event)*mEventLoop->mSetSize);
    if (mEventState->events == NULL) {
        LOG(LL_ERROR, "malloc event state.events error:(%s).",
            strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

void CEventPoll::ApiFree() {
    DEBUG(LL_ALL, "CEventPoll::ApiFree():Begin");
    if (mEventState != NULL) {
        free(mEventState);
        mEventState = NULL;
    }
    DEBUG(LL_ALL, "CEventPoll::ApiFree():End");
}

int CEventPoll::ApiAddEvent(int aSockId, int aAddMask) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), mask:(%d).",
        aSockId, aAddMask);
    if (aSockId <= 2) {
        return RET_ERROR;
    }
    struct epoll_event ee;
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    int op = mEventLoop->mFileEvent[aSockId].mMask == AE_NONE ?
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    aAddMask |= mEventLoop->mFileEvent[aSockId].mMask; /* Merge old events */
    mEventLoop->mFileEvent[aSockId].mMask = aAddMask;
    if (aAddMask & AE_READABLE) ee.events |= EPOLLIN;
    if (aAddMask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = aSockId;
    if (epoll_ctl(mEventState->epfd, op, aSockId, &ee) == RET_ERROR) {
        DEBUG(LL_ERROR, "epoll_ctl error:(%s).",
            strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), mask:(%d).",
        aSockId, aDelMask);
    struct epoll_event ee;
    ee.events = 0;
    if (aDelMask == AE_NONE)    ee.events |= EPOLLIN | EPOLLOUT;
    if (aDelMask & AE_READABLE) ee.events |= EPOLLIN;
    if (aDelMask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = aSockId;
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an DEL operation. */
    int op = aDelMask == AE_NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

    /* Note, Kernel < 2.6.9 requires a non null event pointer even for
     * EPOLL_CTL_DEL. */
    if (epoll_ctl(mEventState->epfd, op, aSockId, &ee) == RET_ERROR) {
        DEBUG(LL_ERROR, "epoll_ctl error:(%s).",
            strerror(errno));
        return;
    }
    DEBUG(LL_ALL, "End");
}

int CEventPoll::ApiPollWait(struct timeval *aTvp) {
    DEBUG(LL_ALL, "Begin");
    int retval, numevents = 0;

    retval = epoll_wait(mEventState->epfd,
            mEventState->events, mEventLoop->mSetSize,
            aTvp ? (aTvp->tv_sec*1000 + aTvp->tv_usec/1000) : -1);
    if (retval > 0) {
        int j;

        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct epoll_event *e = mEventState->events+j;

            if (e->events & EPOLLIN) {
                DEBUG(LL_DBG, "read sockid:(%d) set is ok.",
                    e->data.fd);
                mask |= AE_READABLE;
            }
            if (e->events & EPOLLOUT) {
                DEBUG(LL_DBG, "write sockid:(%d) set is ok.",
                    e->data.fd);
                mask |= AE_WRITABLE;
            }
            if (e->events & (EPOLLERR|EPOLLHUP)) {
                DEBUG(LL_DBG, "err sockid:(%d) set is ok.",
                    e->data.fd);
                mask |= AE_RWERROR;
            }
            mEventLoop->mFiredEvent[j].mSockId = e->data.fd;
            mEventLoop->mFiredEvent[j].mMask   = mask;
        }
    } else if (retval == 0) {
        LOG(LL_INFO, "epoll time out.");
    } else {
        LOG(LL_ERROR, "epoll error:(%s).", strerror(errno));
    }
    LOG(LL_INFO, "fired num:(%d).", numevents);
    DEBUG(LL_ALL, "End");
    return numevents;
}

const char* CEventPoll::ApiName() {
    return "epoll";
}
