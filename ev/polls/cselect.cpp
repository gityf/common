/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CEventPoll by api select.
*/
#include <string.h>
#include <sys/select.h>
#include <stdlib.h>
#include <errno.h>
#include "base/logwriter.h"
typedef struct SEventState {
    fd_set rfds, wfds;
    /* We need to have a copy of the fd sets as it's not safe to reuse
     * FD sets after select(). */
    fd_set _rfds, _wfds;
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
    FD_ZERO(&mEventState->rfds);
    FD_ZERO(&mEventState->wfds);
    FD_ZERO(&mEventState->_rfds);
    FD_ZERO(&mEventState->_wfds);
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
    // do not add fd=0 into select to avoid while(1) in ApiPollWait.
    if (aSockId <= 2) {
        return RET_ERROR;
    }
    aAddMask |= mEventLoop->mFileEvent[aSockId].mMask; /* Merge old events */
    mEventLoop->mFileEvent[aSockId].mMask = aAddMask;

    if (aAddMask & AE_READABLE) FD_SET(aSockId, &mEventState->rfds);
    if (aAddMask & AE_WRITABLE) FD_SET(aSockId, &mEventState->wfds);
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), mask:(%d).",
        aSockId, aDelMask);
    FD_CLR(aSockId, &mEventState->rfds);
    FD_CLR(aSockId, &mEventState->wfds);
    if (aDelMask & AE_READABLE) FD_SET(aSockId, &mEventState->rfds);
    if (aDelMask & AE_WRITABLE) FD_SET(aSockId, &mEventState->wfds);
    DEBUG(LL_ALL, "End");
}

int CEventPoll::ApiPollWait(struct timeval *aTvp) {
    DEBUG(LL_ALL, "Begin");
    int retval, j, numevents = 0;

    memcpy(&mEventState->_rfds, &mEventState->rfds, sizeof(fd_set));
    memcpy(&mEventState->_wfds, &mEventState->wfds, sizeof(fd_set));

    LOG(LL_INFO, "try select with max sockid:(%d)...",
        mEventLoop->mMaxSockId);
    #ifdef HPUX
    retval = select(mEventLoop->mMaxSockId+1,
                reinterpret_cast<int *>(&mEventState->_rfds),
                reinterpret_cast<int *>(&mEventState->_wfds),
                NULL, aTvp);
    #else
    retval = select(mEventLoop->mMaxSockId+1,
                &mEventState->_rfds,
                &mEventState->_wfds,
                NULL, aTvp);
    #endif
    if (retval > 0) {
        for (j = 3; j <= mEventLoop->mMaxSockId; j++) {
            int mask = AE_NONE;
            SFileEvent *fe = &mEventLoop->mFileEvent[j];

            if (fe->mMask == AE_NONE) continue;
            /*
            if ((fe->mMask & AE_READABLE) && FD_ISSET(j,&mEventState->_rfds))
            {
                DEBUG(LL_DBG, "read sockid:(%d) set is ok.", j);
                mask |= AE_READABLE;
            }
            if ((fe->mMask & AE_WRITABLE) && FD_ISSET(j,&mEventState->_wfds))
            {
                DEBUG(LL_DBG, "write sockid:(%d) set is ok.", j);
                mask |= AE_WRITABLE;
            }
            */
            if (FD_ISSET(j, &mEventState->_rfds)) {
                DEBUG(LL_DBG, "read sockid:(%d) set is ok.", j);
                mask |= AE_READABLE;
            }
            if (FD_ISSET(j, &mEventState->_wfds)) {
                DEBUG(LL_DBG, "write sockid:(%d) set is ok.", j);
                mask |= AE_WRITABLE;
            }
            if (mask != AE_NONE) {
                mEventLoop->mFiredEvent[numevents].mSockId = j;
                mEventLoop->mFiredEvent[numevents].mMask   = mask;
                numevents++;
            }
            DEBUG(LL_DBG, "fired event sockid:(%d),mask:(%d).",
                j, mask);
        }
    } else if (retval == 0) {
        LOG(LL_INFO, "select time out.");
    } else {
        LOG(LL_ERROR, "select error:(%s).", strerror(errno));
        FD_ZERO(&mEventState->rfds);
        FD_ZERO(&mEventState->wfds);
        FD_ZERO(&mEventState->_rfds);
        FD_ZERO(&mEventState->_wfds);
        mEventLoop->mMaxSockId = 0;
    }
    LOG(LL_INFO, "fired num:(%d).", numevents);
    DEBUG(LL_ALL, "End");
    return numevents;
}

const char* CEventPoll::ApiName() {
    return "select";
}
