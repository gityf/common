/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class EventLoop.
*/
#ifndef _COMMON_EV_CEVENTLOOP_
#define _COMMON_EV_CEVENTLOOP_
/*
* return code defination.
*/
#define RET_OK      0
#define RET_ERROR  -1
#define AE_NONE     0
#define AE_READABLE 1
#define AE_WRITABLE 2
#define AE_RWERROR  4


#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT   4
#define AE_WAIT_FOREVER 8
#define AE_WAIT_MS      500

class CEventPoll;
/* A fired event */
typedef struct SFileEvent {
    int mSockId;
    int mMask;
} SFileEvent;
class FileEventHandler;
class EventLoop {
 public:
    EventLoop();
    ~EventLoop();

    int CreateEventLoop(int aSetSize);
    void DeleteEventLoop();
    void Stop();
    void SetFileEventHandler(FileEventHandler *aFEHandler);
    int AddFileEvent(int aSockId, int aMask);
    void DeleteFileEvent(int aSockId, int aMask);
    int GetFileEvents(int aSockId);
    int ProcessEvents(int aEventFlag, int aWaitTime);
    void MainLoop(int aWaitTime);

    SFileEvent *mFileEvent;
    SFileEvent *mFiredEvent;
    int mMaxSockId;
    int mSetSize;
 private:
    FileEventHandler *mFileEventHandler;
    CEventPoll *mEventPoll;
    bool mIsInitialed;
};
#endif  // _COMMON_EV_CEVENTLOOP_
