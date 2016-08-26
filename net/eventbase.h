#pragma once
#include <map>
#include <vector>
#include <set>
#include <atomic>
#include "ev/eventloop.h"
#include "ev/eventhandler.h"

enum EventType {
    kRead,
    kWrite,
    kConnectError
};
using EventFuncPtr = std::function<int(int)>;
// event idle callback function.
using IdleFuncPtr = std::function<void()>;
static const int kLoopSize = 4096;
// one buffer event for one thread.
class EventBase : public FileEventHandler {
public:
    EventBase();
    ~EventBase();
    // To do initialize.
    bool initData(int aLoopSize = kLoopSize);
    // loopforever
    void loop();
    void stop();

    // buffer of reading/writing/error function for socket io.
    void Request(int aSockid) override;
    void Response(int aSockid) override;
    void ErrSocket(int aSockid) override;
    // enable fd to read/write.
    void enableRead(int aSockid, bool aIsEnable = true);
    void enableWrite(int aSockid, bool aIsEnable = true);

    // complete check function.
    void setRWCallback(EventType aType, EventFuncPtr&& aCallback) {
        switch (aType)
        {
        case kRead:
            read_ = std::move(aCallback);
            break;
        case kWrite:
            write_ = std::move(aCallback);
            break;
        case kConnectError:
            error_ = std::move(aCallback);
        default:
            break;
        }
    }
    void setRWCallback(EventType aType, const EventFuncPtr& aCallback) {
        setRWCallback(aType, EventFuncPtr(aCallback));
    }

    // one fd one callback function.
    void setCallback(int aSockid, EventFuncPtr&& aCallback) {
        readPairs_[aSockid] = std::move(aCallback);
    }
    void setCallback(int aSockid, const EventFuncPtr& aCallback) {
        setCallback(aSockid, EventFuncPtr(aCallback));
    }

    // set idle callback function.
    void setIdleCallback(IdleFuncPtr&& aCallback) {
        idle_ = std::move(aCallback);
    }
    void setIdleCallback(const IdleFuncPtr& aCallback) {
        setIdleCallback(IdleFuncPtr(aCallback));
    }

    // get eventloop pointer.
    EventLoop* getEventLoop() {
        return eventLoop_;
    }
public:
    EventLoop* eventLoop_;
private:
    // array of socket time out.
    std::map<int, long> mSockIdTimerPair;
    EventFuncPtr read_, write_, error_;
    std::map<int, EventFuncPtr> readPairs_;
    IdleFuncPtr idle_;
    // stop flag.
    bool isStop_;
};
// end of local file.
