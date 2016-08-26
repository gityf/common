#include <thread>
#include "eventbase.h"
#include "base/log.h"

EventBase::EventBase() {
    eventLoop_ = NULL;
    isStop_ = false;
    read_ = nullptr;
    idle_ = nullptr;
}
EventBase::~EventBase() {
    delete eventLoop_;
}

bool EventBase::initData(int aLoopSize) {
    // create eventbase loop.
    eventLoop_ = new EventLoop();
    if (RET_ERROR == eventLoop_->CreateEventLoop(aLoopSize)) {
        return false;
    }
    eventLoop_->SetFileEventHandler(this);
    isStop_ = false;
    return true;
}

void EventBase::loop() {
    while (!isStop_) {
        eventLoop_->MainLoop(100);
        if (idle_) {
            idle_();
        }
    }
}

void EventBase::stop() {
    isStop_ = true;
}

void EventBase::Request(int aSockid) {
    if (readPairs_.find(aSockid) != readPairs_.end()) {
        if (RET_ERROR == readPairs_[aSockid](aSockid)) {
            ErrSocket(aSockid);
        }
    }
    else {
        if (read_) {
            if (RET_ERROR == read_(aSockid)) {
                ErrSocket(aSockid);
            }
        }
    }
}

void EventBase::Response(int aSockid) {
    if (write_) {
        if (RET_ERROR == write_(aSockid)) {
            ErrSocket(aSockid);
        }        
    }
}

void EventBase::ErrSocket(int aSockid) {
    eventLoop_->DeleteFileEvent(aSockid,
        AE_WRITABLE | AE_READABLE | AE_RWERROR);
    close(aSockid);
    if (error_) {
        error_(aSockid);
    }
}

void EventBase::enableRead(int aSockid, bool aIsEnable /* = true */) {
    if (aIsEnable) {
        eventLoop_->AddFileEvent(aSockid, AE_READABLE);
    }
    else {
        eventLoop_->DeleteFileEvent(aSockid, AE_READABLE);
    }
}

void EventBase::enableWrite(int aSockid, bool aIsEnable /* = true */) {
    if (aIsEnable) {
        eventLoop_->AddFileEvent(aSockid, AE_WRITABLE);
    }
    else {
        eventLoop_->DeleteFileEvent(aSockid, AE_WRITABLE);
    }
}
