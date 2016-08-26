#pragma once
#include <string>
#include <vector>
#include <memory>
#include "bufferevent.h"
#include "eventbase.h"
#include "socket.h"

// callback for message handle.
using TcpCallbackPtr = std::function<int(int aSockid, Slice& aRequ, Buffer& aResp)>;
// callback for connection and disconnection.
using TcpConnectionPtr = std::function<void(int fd)>;
// type of connection and disconnection.
enum EConnType {
    kConnect,
    kDisconnect
};
// Tcp Server
struct TcpServer {
    TcpServer();
    ~TcpServer();
    // initialize.
    int initData();
    // init callback functions
    int initCallBack();
    // start tcp server.
    void start();
    // return 0 on sucess, errno on error
    int bind(const std::string& host, short port, int aFamily = AF_INET);
    // accept callback and others are set.
    int setAcceptInfo(int aListenFd);
    // accept.
    int handleAccept(int aSockId);
    // get bufferevent ptr.
    BufferEvent* getBufferEvent() {
        return bufferEvent_.get(); 
    }
    // get eventbase ptr.
    EventBase* getEventBase() {
        return eventBase_.get();
    }
    // message is incoming.
    void onRequest(int aSockid, Slice& aBuf);

    // new connection is incomig.
    void onConnect(int aSockid);

    // disconnection is incoming.
    void onDisconnect(int aSockid);

    // user can set the tcp callback function here.
    void setTcpCallback(const TcpCallbackPtr& aCallback) {
        setTcpCallback(TcpCallbackPtr(aCallback));
    }
    void setTcpCallback(TcpCallbackPtr&& aCallback) {
        tcpCallback_ = std::move(aCallback);
    }
    // user can set the tcp connection callback function here.
    void setConnCallback(EConnType aConnType, const TcpConnectionPtr& aCallback) {
        setConnCallback(aConnType, TcpConnectionPtr(aCallback));
    }
    void setConnCallback(EConnType aConnType, TcpConnectionPtr&& aCallback) {
        switch (aConnType)
        {
        case kConnect:
            connectCallback_ = std::move(aCallback);
            break;
        case kDisconnect:
            disconnectCallback_ = std::move(aCallback);
            break;
        default:
            break;
        }
    }
private:
    std::shared_ptr<BufferEvent> bufferEvent_;
    std::shared_ptr<EventBase> eventBase_;
    std::shared_ptr<Socket> socket_;
    // read callback function ptr.
    TcpCallbackPtr tcpCallback_;
    // new connection callback function ptr.
    TcpConnectionPtr connectCallback_, disconnectCallback_;
};
// end of local file.
