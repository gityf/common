#include <functional>
#include <iostream>
#include "tcpserver.h"
#include "base/log.h"
#include "base/this_thread.h"

using namespace std::placeholders;

TcpServer::TcpServer() {
}

TcpServer::~TcpServer() {

}
int TcpServer::initData() {
    bufferEvent_.reset(new BufferEvent());
    socket_.reset(new Socket());
    eventBase_.reset(new EventBase());
    eventBase_->initData();
    return RET_OK;
}

int TcpServer::initCallBack() {
    // set read callback
    eventBase_->setRWCallback(kRead, 
        [&](int aSockid) {
        Slice msg;
        int ret = bufferEvent_->Request(aSockid, msg);
        if (ret == RET_ERROR) {
            return RET_ERROR;
        } else if (ret == 1) {
            onRequest(aSockid, msg);
        }
        return RET_OK;
    });
    // set write callback.
    eventBase_->setRWCallback(kWrite,
        [&](int aSockid) {
        return bufferEvent_->Response(aSockid);
    });
    // set disconnection callback.
    eventBase_->setRWCallback(kConnectError,
        [&](int aSockid) {
        onDisconnect(aSockid);
        return RET_OK;
    });
    return RET_OK;
}

void TcpServer::start() {
    eventBase_->loop();
}

int TcpServer::bind(const std::string& host, short port, int aFamily) {
    SAddrInfo addrinfo;
    addrinfo.mAddr = host;
    addrinfo.mPort = port;
    addrinfo.mFamily = aFamily;
    addrinfo.mProtocol = TCP;
    if (RET_ERROR == socket_->TcpServer(&addrinfo)) {
        return RET_ERROR;
    }
    //setAcceptInfo(addrinfo.mSockId);
    return addrinfo.mSockId;
}

int TcpServer::setAcceptInfo(int aListenFd) {
    eventBase_->setCallback(aListenFd,
        std::bind(&TcpServer::handleAccept, this, _1));
    eventBase_->enableRead(aListenFd);
    return RET_OK;
}

int TcpServer::handleAccept(int aSockId) {
    SAddrInfo clnAddr;
    DEBUG(LL_DBG, "start to accept trdid:[%lu].", ThisThread::GetThreadId());
    int ret = socket_->AcceptConnection(aSockId, &clnAddr);
    if (ret == RET_OK) {
        // buffer event only working at non block socket mode.
        socket_->setNonBlock(clnAddr.mSockId, true);
        // enable readable of the client connection.
        eventBase_->enableRead(clnAddr.mSockId);
        onConnect(clnAddr.mSockId);
    }
    return ret;
}

void TcpServer::onRequest(int aSockid, Slice& aBuf) {
    if (tcpCallback_) {
        //eventBase_->enableRead(aSockid, false);
        Buffer& resp = bufferEvent_->getRespBuffer(aSockid);
        if (RET_ERROR == tcpCallback_(aSockid, aBuf, resp)) {
            eventBase_->ErrSocket(aSockid);
            return;
        }
        //eventBase_->enableWrite(aSockid);
        // read event exist as before.
        //eventBase_->enableRead(aSockid);
    }
    else {
        // with out handled function.
        eventBase_->ErrSocket(aSockid);
    }    
}

void TcpServer::onConnect(int aSockid) {
    if (connectCallback_) {
        connectCallback_(aSockid);
    }
}

void TcpServer::onDisconnect(int aSockid) {
    if (disconnectCallback_) {
        disconnectCallback_(aSockid);
    }
}