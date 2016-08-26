#include <sys/poll.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>

typedef struct SEventState {
    struct pollfd* ipfds;
    struct pollfd* iBakpfds;
    int ipfdsSize;
} SEventState;

int CEventPoll::ApiCreate(EventLoop * aEventLoop)
{
    mEventState = reinterpret_cast<SEventState *>(malloc(sizeof(SEventState)));
    if (mEventState == NULL) {
        return RET_ERROR;
    }
    mEventLoop = aEventLoop;
    mEventState->ipfds = (struct pollfd *)malloc(mEventLoop->mSetSize * sizeof(struct pollfd));
    if (!mEventState->ipfds) {
        return RET_ERROR;
    }
    memset(mEventState->ipfds, 0, mEventLoop->mSetSize * sizeof(struct pollfd));
    mEventState->iBakpfds = (struct pollfd *)malloc(mEventLoop->mSetSize * sizeof(struct pollfd));
    if (!mEventState->iBakpfds) {
        return RET_ERROR;
    }
    memset(mEventState->iBakpfds, 0, mEventLoop->mSetSize * sizeof(struct pollfd));
    mEventState->ipfdsSize = 0;
    return RET_OK;
}

void CEventPoll::ApiFree()
{
    if (mEventState != NULL) {
        if (mEventState->ipfds != NULL) {
            free(mEventState->ipfds);
            mEventState->ipfds = NULL;
        }
        if (mEventState->iBakpfds != NULL) {
            free(mEventState->iBakpfds);
            mEventState->iBakpfds = NULL;
        }       
        free(mEventState);
        mEventState = NULL;
    }
}

int CEventPoll::ApiAddEvent(int aSockId, int aAddMask)
{
    if (aSockId <= 0) return RET_ERROR;
    mEventState->ipfds[mEventState->ipfdsSize].fd      = aSockId;
    mEventState->ipfds[mEventState->ipfdsSize].events  = 0;
    mEventState->ipfds[mEventState->ipfdsSize].revents = 0;
    aAddMask |= mEventLoop->mFileEvent[aSockId].mMask; /* Merge old events */
    mEventLoop->mFileEvent[aSockId].mMask = aAddMask;
    
    if (aAddMask & AE_READABLE) {
        mEventState->ipfds[mEventState->ipfdsSize].events |= POLLIN;
    }

    if (aAddMask & AE_WRITABLE) {
        mEventState->ipfds[mEventState->ipfdsSize].events |= POLLOUT;
    }
    mEventState->ipfds[mEventState->ipfdsSize].events |= POLLERR | POLLNVAL | POLLHUP;
    mEventState->ipfdsSize++;
    return RET_OK;
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask)
{
    int ii = 0;
    while (ii < mEventState->ipfdsSize) {
        if (mEventState->ipfds[ii].fd == aSockId) {
            mEventState->ipfdsSize--;
            mEventState->ipfds[ii] = mEventState->ipfds[mEventState->ipfdsSize];
            break;
        }
        ii++;
    }
}

int CEventPoll::ApiPollWait(struct timeval *aTvp)
{
    int retval, j, numevents = 0, fdsize = 0;
    fdsize = mEventState->ipfdsSize;
    memcpy(mEventState->iBakpfds, mEventState->ipfds, fdsize*sizeof(struct pollfd));

    retval = poll(mEventState->iBakpfds, fdsize, 
        aTvp ? (aTvp->tv_sec*1000 + aTvp->tv_usec/1000) : -1);

    if (retval > 0) {
        for (j = 0; j <= fdsize; j++) {
            int mask = 0;
            if (mEventState->iBakpfds[j].revents & POLLIN)  mask |= AE_READABLE;
            if (mEventState->iBakpfds[j].revents & POLLOUT) mask |= AE_WRITABLE;
            if (mEventState->iBakpfds[j].revents & POLLERR) mask |= AE_RWERROR;
            if (mEventState->iBakpfds[j].revents & POLLHUP) mask |= AE_WRITABLE;
            mEventLoop->mFiredEvent[numevents].mSockId = mEventState->iBakpfds[j].fd;
            mEventLoop->mFiredEvent[numevents].mMask   = mask;
            numevents++;
        }
    }
    return numevents;
}

const char *CEventPoll::ApiName() {
    return "poll";
}
