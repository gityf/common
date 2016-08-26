/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class EventLoop.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "eventloop.h"
#include "eventhandler.h"
#include "eventpoll.h"
#include "base/log.h"
EventLoop::EventLoop() {
    DEBUG(LL_ALL, "Begin");
    mFiredEvent = NULL;
    mFileEvent  = NULL;
    mEventPoll  = NULL;
    mSetSize = 0;
    mMaxSockId = 0;
    mIsInitialed = false;
    DEBUG(LL_ALL, "End");
}

EventLoop::~EventLoop() {
    DEBUG(LL_ALL, "Begin");
    DeleteEventLoop();
    DEBUG(LL_ALL, "End");
}

int EventLoop::CreateEventLoop(int aSetSize) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "set size:(%d).", aSetSize);
    int ret = RET_ERROR;
    mSetSize = aSetSize;
    LOG(LL_INFO, "try malloc fire event.");
    mFiredEvent =
        reinterpret_cast<SFileEvent *>(malloc(sizeof(SFileEvent)*mSetSize));
    if (mFiredEvent == NULL) {
        LOG(LL_ERROR, "malloc fire event error:(%s).",
            strerror(errno));
        return RET_ERROR;
    }
    LOG(LL_INFO, "try malloc file event.");
    mFileEvent =
        reinterpret_cast<SFileEvent *>(malloc(sizeof(SFileEvent)*mSetSize));
    if (mFileEvent == NULL) {
        LOG(LL_ERROR, "malloc file event error:(%s).",
            strerror(errno));
        free(mFiredEvent);
        mFiredEvent = NULL;
        return RET_ERROR;
    }
    mFileEventHandler = NULL;
    LOG(LL_INFO, "try zero file and fire event.");
    memset(mFiredEvent, 0, sizeof(SFileEvent) * mSetSize);
    memset(mFileEvent,  0, sizeof(SFileEvent) * mSetSize);
    int ii = 0;
    LOG(LL_INFO, "try initializing file and fire event.");
    while (ii < mSetSize) {
        mFileEvent[ii].mMask   = AE_NONE;
        mFileEvent[ii].mSockId = 0;
        ii++;
    }
    try {
        LOG(LL_INFO, "try new CEventPoll.");
        mEventPoll = new CEventPoll();
    } catch (...) {
        LOG(LL_ERROR, "new CEventPoll failed.");
        free(mFiredEvent);
        mFiredEvent = NULL;
        free(mFileEvent);
        mFileEvent = NULL;
        return RET_ERROR;
    }
    if (mEventPoll == NULL) {
        LOG(LL_ERROR, "mEventPoll is null.");
        free(mFiredEvent);
        mFiredEvent = NULL;
        free(mFileEvent);
        mFileEvent = NULL;
        return RET_ERROR;
    }
    LOG(LL_INFO, "try CEventPoll.ApiCreate.");
    ret = mEventPoll->ApiCreate(this);
    if (ret != RET_OK) {
        LOG(LL_ERROR, "ApiCreate failed.");
        free(mFiredEvent);
        mFiredEvent = NULL;
        free(mFileEvent);
        mFileEvent = NULL;
        delete mEventPoll;
        mEventPoll = NULL;
        return ret;
    }
    LOG(LL_WARN, "CEventPoll.(%s) is using...",
        mEventPoll->ApiName());
    mIsInitialed = true;
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

void EventLoop::DeleteEventLoop() {
    DEBUG(LL_ALL, "Begin");
    DEBUG(LL_INFO, "try to free fire and file event.");
    if (mFiredEvent != NULL) {
        free(mFiredEvent);
        mFiredEvent = NULL;
    }
    if (mFileEvent != NULL) {
        free(mFileEvent);
        mFileEvent = NULL;
    }
    DEBUG(LL_INFO, "try to free ApiPoll event.");
    if (mEventPoll != NULL) {
        mEventPoll->ApiFree();
        free(mEventPoll);
        mFiredEvent = NULL;
    }
    DEBUG(LL_INFO, "try to zero file event handler.");
    if (mFileEventHandler != NULL) {
        mFileEventHandler = NULL;
    }
    mSetSize = 0;
    mMaxSockId = 0;
    mIsInitialed = false;
    DEBUG(LL_ALL, "End");
}

void EventLoop::Stop() {
    LOG(LL_INFO, "EventLoop::Stop():Begin");
    LOG(LL_INFO, "EventLoop::Stop():End");
}

void EventLoop::SetFileEventHandler(FileEventHandler *aFEHandler) {
    mFileEventHandler = aFEHandler;
}

int EventLoop::AddFileEvent(int aSockId, int aMask) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_DBG, "sockid:(%d),mask:(%d).",
        aSockId, aMask);
    if (!mIsInitialed) {
        LOG(LL_ERROR, "is not initialed.");
        return RET_ERROR;
    }
    if (aSockId >= mSetSize) {
        errno = ERANGE;
        LOG(LL_ERROR, "to large sockid:(%d),max:(%d),error:(%s).",
            aSockId, mSetSize, strerror(errno));
        return RET_ERROR;
    }

    DEBUG(LL_DBG, "try add event poll.");
    if (RET_ERROR == mEventPoll->ApiAddEvent(aSockId, aMask)) {
        LOG(LL_ERROR, "ApiAddEvent failed sockid:(%d), mask:(%d).",
            aSockId, aMask);
        return RET_ERROR;
    }
    mFileEvent[aSockId].mSockId = aSockId;
    DEBUG(LL_VARS, "sockid:(%d), new mask:(%d).",
        aSockId, mFileEvent[aSockId].mMask);

    if (aSockId > mMaxSockId) {
        mMaxSockId = aSockId;
        LOG(LL_INFO, "max sockid:(%d)", mMaxSockId);
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

void EventLoop::DeleteFileEvent(int aSockId, int aMask) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_DBG, "sockid:(%d),mask:(%d).", aSockId, aMask);
    if (!mIsInitialed) {
        LOG(LL_ERROR, "is not initialed.");
        return;
    }
    if (aSockId >= mSetSize) return;
    if (mFileEvent[aSockId].mMask == AE_NONE) return;
    mFileEvent[aSockId].mMask &= ~aMask;
    DEBUG(LL_VARS, "max sockid:(%d),new mask is:(%d).",
        mMaxSockId, mFileEvent[aSockId].mMask);
    if (aSockId == mMaxSockId && mFileEvent[aSockId].mMask == AE_NONE) {
        /* Update the max fd */
        int j;
        for (j = mMaxSockId-1; j >= 3; j--)
            if (mFileEvent[j].mMask != AE_NONE) break;
        mMaxSockId = j;
        LOG(LL_VARS, "update max sockid:(%d).", mMaxSockId);
    }
    DEBUG(LL_INFO, "try delete event from poll.");
    mEventPoll->ApiDelEvent(aSockId, mFileEvent[aSockId].mMask);
    DEBUG(LL_ALL, "End");
}

int EventLoop::GetFileEvents(int aSockId) {
    if (!mIsInitialed) {
        LOG(LL_ERROR, "EventLoop::GetFileEvents is not initialed.");
        return RET_ERROR;
    }
    if (aSockId >= mSetSize) return 0;

    return mFileEvent[aSockId].mMask;
}

int EventLoop::ProcessEvents(int aEventFlag, int aWaitTime) {
    DEBUG(LL_ALL, "Begin");
    int processed = 0, numevents;

    /* Nothing to do? return ASAP */
    if (!(aEventFlag & AE_FILE_EVENTS)) return 0;

    /* Note that we want call select() even if there are no
     * file events to process as long as we want to process time
     * events, in order to sleep until the next time event is ready
     * to fire. */
    if (mMaxSockId != 0 || !(aEventFlag & AE_DONT_WAIT)) {
        int j;
        struct timeval tv, *tvp;
        /* If we have to check for events but need to return
         * ASAP because of AE_DONT_WAIT we need to set the timeout
         * to zero
         */
        tvp = NULL;
        if (aEventFlag & AE_DONT_WAIT) {
            tv.tv_sec = tv.tv_usec = 0;
            tvp = &tv;
        } else if (aEventFlag & AE_WAIT_FOREVER) {
            tvp = NULL; /* wait forever */
        } else {
            tv.tv_sec = aWaitTime / 1000;
            tv.tv_usec = (aWaitTime % 1000) * 1000;
            tvp = &tv;
        }
        numevents = mEventPoll->ApiPollWait(tvp);
        for (j = 0; j < numevents; j++) {
            int mask = mFiredEvent[j].mMask;
            int fd   = mFiredEvent[j].mSockId;
            int rfired = 0;

            /* note the fe->mask & mask & ... code: maybe an already processed
             * event removed an element that fired and we still didn't
             * processed, so we check if the event is still valid. */
            LOG(LL_DBG, "fireMask=(%d),fileMask=(%d), ",
                mask, mFileEvent[fd].mMask);

            if (mFileEvent[fd].mMask & mask & AE_RWERROR) {
                mFileEventHandler->ErrSocket(fd);
            }
            if (mFileEvent[fd].mMask & mask & AE_READABLE) {
                rfired = 1;
                mFileEventHandler->Request(fd);
            }
            if (mFileEvent[fd].mMask & mask & AE_WRITABLE) {
                if (!rfired) mFileEventHandler->Response(fd);
            }
            processed++;
        }
    }

    DEBUG(LL_ALL, "End");
    return processed; /* return the number of processed file events */
}

void EventLoop::MainLoop(int aWaitTime) {
    if (!mIsInitialed) {
        LOG(LL_ERROR, "EventLoop::MainLoop is not initialed.");
        return;
    }
    ProcessEvents(AE_ALL_EVENTS, aWaitTime);
}
// end of local file.
