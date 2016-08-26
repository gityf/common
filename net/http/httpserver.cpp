#include <functional>
#include <iostream>
#include "base/log.h"
#include "base/this_thread.h"
#include "net/httputils.h"
#include "httpserver.h"
#include "net/httpcodec.h"
using namespace std::placeholders;
namespace {
    bool parseLine(const char* begin, const char* end, HttpRequest* request) {
        bool succeed = false;
        const char* start = begin;
        const char* space = std::find(start, end, ' ');
        if (space != end && request->setMethod(start, space)) {
            start = space + 1;
            space = std::find(start, end, ' ');
            if (space != end) {
                const char* question = std::find(start, space, '?');
                if (question != space) {
                    request->setPath(start, question);
                    request->setQuery(question, space);
                    (static_cast<common::HttpUtils*>(0))->ParseParams(request->query(),
                        request->mutable_params());
                }
                else {
                    request->setPath(start, space);
                }
                start = space + 1;
                succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
                if (succeed) {
                    if (*(end - 1) == '1') {
                        request->setVersion(HttpRequest::kHttp11);
                    }
                    else if (*(end - 1) == '0') {
                        request->setVersion(HttpRequest::kHttp10);
                    }
                    else {
                        succeed = false;
                    }
                }
            }
        }
        return succeed;
    }

    bool parseHeader(const char* begin, const char* end, HttpRequest* request) {
        string header(begin, end);
        (static_cast<common::HttpUtils*>(0))->ParseHeaders(header, request->mutable_headers());
        return true;
    }

    void setBody(const char* begin, const char* end, HttpRequest* request) {
        request->setBody(begin, end);
    }
}

HttpServer::HttpServer(TcpServer* aTcpServer) {
    tcpserver_ = aTcpServer;
}

HttpServer::~HttpServer() {

}

int HttpServer::initData(const string& ip, int port) {
    tcpserver_->initData();
    tcpserver_->initCallBack();
    tcpserver_->getBufferEvent()->setCodec(new HttpCodec);
    int listenfd = tcpserver_->bind(ip, port);
    tcpserver_->setAcceptInfo(listenfd);
}

void HttpServer::start() {
    tcpserver_->setTcpCallback(std::bind(&HttpServer::onRequest,
        this, _1, _2, _3));
    tcpserver_->start();
}

int HttpServer::onRequest(int aSockid, Slice& aRequ, Buffer& aResp) {
    if (httpCallback_) {
        HttpRequest requ;
        HttpResponse resp(true);
        parseRequest(aRequ, &requ);
        if (RET_ERROR == httpCallback_(requ, &resp)) {
            return RET_ERROR;
        }
        resp.appendToBuffer(aResp);
    }
    else {
        return RET_ERROR;
    }
}

int HttpServer::parseRequest(Slice& aRequ, HttpRequest* aOutHttpRequ) {
    parseLine(aRequ.begin(), aRequ.lineEnd(), aOutHttpRequ);
    parseHeader(aRequ.lineEnd() + 2, aRequ.bodyBegin() - 4, aOutHttpRequ);
    aOutHttpRequ->setBody(aRequ.bodyBegin(), aRequ.bodyBegin() + aRequ.bodyLen());
    aOutHttpRequ->setBodyLength(aRequ.bodyLen());
}
