/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class HttpClient.
*/
#ifndef _COMMON_CHTTPCLIENT_H_
#define _COMMON_CHTTPCLIENT_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "socket.h"
#include "httputils.h"
using namespace std;

namespace common {
    class HttpClient {
    public:
        // enum error message.
        enum EHttpErrCode {
            // OK OK
            kOkHttpOK = 0,
            // socket error
            kErrCreateSocket,
            // can not connect to server.
            kErrConnServer,
            // send request failed to server.
            kErrSendToServer,
            // time-out is coming.
            kErrRespTimeout,
            // bad http server address.
            kErrBadHttpAddress,
            // http request error.
            kErrRequNull,
            // http response is null.
            kErrRespNull,
            // bad style of response.
            kErrInvalidResponse,
            // bad http response status.
            kErrRespStatus,
            // unknown error.
            kErrUnknown
        };
        static const int kDefaultTimeoutMs = 500;
        // constructor.
        HttpClient() {
            mIsInited = false;
        };
        // construct and run HTTP request.
        HttpClient(const string& aUrl, const string& aMethod = "GET",
                    const int aTimeout = kDefaultTimeoutMs) {
            mIsInited = false;
            Request(aUrl, aMethod, aTimeout);
        }
        // destructor.
        virtual ~HttpClient() {}
        // initial operation.
        bool InitData();
        // connect, send, recv and parse.
        bool SimpleHttpRequ(SAddrInfo* aAddrInfo,
                            string &aResponse, const int aTimeout);
        // to parse url string to struct urlinfo
        bool UrlInfo(const string &aUrl, SUrlInfo* aOutUrlInfo);
        // execute HTTP request.
        bool Request(const string& aUrl,
            const string& aMethod = "GET",
            const int aTimeout = kDefaultTimeoutMs);
        // post method
        bool OnlyPost(const string& aUrl, const string& aBody, int& aSockId);
        // get method
        bool OnlyGet(const string& aUrl, int& aSockId, int aTimeout = kDefaultTimeoutMs);
        // post with response
        bool Post(const string& aUrl, const string &aInBody,
            int &aSockId, string& aOutBody, int aTimeout = kDefaultTimeoutMs);
        // get with response
        bool Get(const string& aUrl, int &aSockId,
            string& aOutBody, int aTimeout = kDefaultTimeoutMs);
        // receive response.
        bool RecvResp(int& aSockId, string& aOutBody, int aTimeout);

        // URL is exist or not.
        bool IsUrlExist(const string& aUrl);
        // HTTP request is Done.
        bool Done() const;
        // get the error message.
        string ErrDesc() const;
        // get httputils pointer
        HttpUtils* GetHttpUtils() {
            return mHttpUtils.get();
        }
        std::shared_ptr<HttpUtils> mHttpUtils;
    private:
        EHttpErrCode mErrCode;           // current error code
        std::shared_ptr<Socket> mSocket;
        bool mIsInited;
    };

} // namespace

#endif //_COMMON_CHTTPCLIENT_H_

