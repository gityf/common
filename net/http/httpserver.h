#pragma once
#include <string>
#include <vector>
#include <memory>
#include "net/tcpserver.h"
#include "httprequest.h"
#include "httpresponse.h"

using HttpCallbackPtr = std::function<int(const HttpRequest& aRequ, HttpResponse* aResp)>;
// Tcp Server
struct HttpServer {
    HttpServer(TcpServer* aTcpServer);
    ~HttpServer();
    int initData(const string& ip, int port);
    // start http server.
    void start();
    // callback for tcpserver
    int onRequest(int aSockid, Slice& aRequ, Buffer& aResp);
    // user can set the tcp callback function here.
    void setHttpCallback(const HttpCallbackPtr& aCallback) {
        setHttpCallback(HttpCallbackPtr(aCallback));
    }
    void setHttpCallback(HttpCallbackPtr&& aCallback) {
        httpCallback_ = std::move(aCallback);
    }
    int parseRequest(Slice& aRequ, HttpRequest* aOutRequ);
private:
    // tcp server handler.
    TcpServer* tcpserver_;
    // http callback.
    HttpCallbackPtr httpCallback_;
};
// end of local file.
