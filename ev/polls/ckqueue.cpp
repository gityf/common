#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

typedef struct SEventState {
    int kqfd;
    struct kevent *events;
} SEventState;

int CEventPoll::ApiCreate(EventLoop * aEventLoop)
{
    mEventState = (SEventState *)malloc(sizeof(SEventState));
    if (mEventState == NULL) {
        return RET_ERROR;
    }
    mEventLoop = aEventLoop;
    mEventState->kqfd = kqueue();
    if (mEventState->kqfd == RET_ERROR) {
        return RET_ERROR;
    }

    return RET_OK;
}

void CEventPoll::ApiFree()
{
    if (mEventState != NULL) {
        free(mEventState);
        mEventState = NULL;
    }
}

int CEventPoll::ApiAddEvent(int aSockId, int aAddMask)
{
    if (aSockId <= 0) {
        return RET_ERROR;
    }
    aAddMask |= mEventLoop->mFileEvent[aSockId].mMask; /* Merge old events */
    mEventLoop->mFileEvent[aSockId].mMask = aAddMask;
    struct kevent ke;

    if (aAddMask & AE_READABLE) {
        EV_SET(&ke, aSockId, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(mEventState->kqfd, &ke, 1, NULL, 0, NULL) == RET_ERROR)
            return RET_ERROR;
    }
    if (aAddMask & AE_WRITABLE) {
        EV_SET(&ke, aSockId, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if (kevent(mEventState->kqfd, &ke, 1, NULL, 0, NULL) == RET_ERROR)
            return RET_ERROR;
    }
    return RET_OK;
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask)
{
    struct kevent ke;

    if (aDelMask & AE_READABLE) {
        EV_SET(&ke, aSockId, EVFILT_READ, EV_ADD, 0, 0, NULL);
    } else {
        EV_SET(&ke, aSockId, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    }
    kevent(mEventState->kqfd, &ke, 1, NULL, 0, NULL);
    if (aDelMask & AE_WRITABLE) {
        EV_SET(&ke, aSockId, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    } else {
        EV_SET(&ke, aSockId, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    }
    kevent(mEventState->kqfd, &ke, 1, NULL, 0, NULL);
}

int CEventPoll::ApiPollWait(struct timeval *aTvp)
{
    int retval, numevents = 0;

    if (aTvp != NULL) {
        struct timespec timeout;
        timeout.tv_sec = aTvp->tv_sec;
        timeout.tv_nsec = aTvp->tv_usec * 1000;
        retval = kevent(mEventState->kqfd, NULL, 0,
            mEventState->events, mEventLoop->mSetSize, &timeout);
    } else {
        retval = kevent(mEventState->kqfd, NULL, 0,
            mEventState->events, mEventLoop->mSetSize, NULL);
    }

    if (retval > 0) {
        int j;

        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct kevent *e = mEventState->events+j;

            if (e->filter == EVFILT_READ)  mask |= AE_READABLE;
            if (e->filter == EVFILT_WRITE) mask |= AE_WRITABLE;
            mEventLoop->mFiredEvent[j].mSockId = e->ident;
            mEventLoop->mFiredEvent[j].mMask   = mask;
        }
    }
    return numevents;
}

const char *CEventPoll::ApiName() {
    return "kqueue";
}
