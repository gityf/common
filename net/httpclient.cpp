/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class HttpClient.
*/
#include <unistd.h>
#include "base/localdef.h"
#include "base/stringutils.h"
#include "httputils.h"
#include "httpclient.h"

using namespace std;

namespace common {
    bool HttpClient::InitData() {
        if (!mIsInited) {
            mHttpUtils.reset(new HttpUtils());
            mSocket.reset(new Socket());
            mIsInited = true;
        }
        return mIsInited;
    }

    bool HttpClient::SimpleHttpRequ(SAddrInfo* aAddrInfo,
        string &aResponse, const int aTimeout) {
        if (!mIsInited) {
            if (!InitData()) {
                return false;
            }
        }

        mErrCode = kOkHttpOK;
        do {
            if (RET_ERROR == mSocket->MakeTcpConn(aAddrInfo)) {
                mErrCode = kErrConnServer;
                break;
            }
            int ret = mSocket->TcpSend(aAddrInfo->mSockId, aAddrInfo->mReqCmd.c_str(),
                aAddrInfo->mReqCmd.length(), aTimeout, false);
            if (RET_ERROR == ret) {
                mErrCode = kErrSendToServer;
                break;
            }
            mSocket->setNonBlock(aAddrInfo->mSockId, true);
            aResponse.resize(MAX_SIZE_2K);
            char *buffPtr = const_cast<char *>(aResponse.data());
            ret = mSocket->RecvHttpPkg(aAddrInfo->mSockId,
                buffPtr, MAX_SIZE_2K, aTimeout);
            if (ret != RET_ERROR) {
                aResponse.resize(ret);
            } else {
                aResponse.clear();
                mErrCode = kErrRespNull;
                break;
            }
        } while(0);
        mSocket->CloseSocket(aAddrInfo->mSockId);
        return mErrCode == kOkHttpOK;
    }

    bool HttpClient::UrlInfo(const string &aUrl, SUrlInfo* aOutUrlInfo) {
        mErrCode = kOkHttpOK;
        if (!mIsInited) {
            if (!InitData()) {
                return false;
            }
        }

        mHttpUtils->ParseUrl(aUrl, *aOutUrlInfo);

        // check params
        string urlParams = mHttpUtils->GetUrlParam();
        if (!urlParams.empty()) {
            if (!aOutUrlInfo->mUrlParams.empty()) {
                aOutUrlInfo->mUrlParams += "&";
            }
            aOutUrlInfo->mUrlParams += urlParams;
        }
        // check host
        if (aOutUrlInfo->mHost.empty()) {
            return false;
        }
        if (mSocket->IsIPv4Addr(aOutUrlInfo->mHost)) {
            aOutUrlInfo->mIpAddress = aOutUrlInfo->mHost;
        } else {
            char ipstr[64] = {0};
            if (mSocket->Name2IPv4(aOutUrlInfo->mHost.c_str(), ipstr)) {
                aOutUrlInfo->mIpAddress = ipstr;
            } else {
                return false;
            }
        }
        return true;
    }

    // ִ��HTTP����
    // \param url HTTP����URL
    // \param method HTTP����Method,Ĭ��Ϊ"GET"
    // \param timeout HTTP����ʱʱ��,��λΪ��,Ĭ��Ϊ5��
    // \retval true ִ�гɹ�
    // \retval false ִ��ʧ��
    bool HttpClient::Request(const string &aUrl,
        const string &aMethod, const int aTimeout) {
        mErrCode = kOkHttpOK;
        if (!mIsInited) {
            if (!InitData()) {
                return false;
            }
        }

        struct SUrlInfo urlInfo;
        if (!UrlInfo(aUrl, &urlInfo)) {
            return false;
        }

        SAddrInfo addrInfo;
        // generate Request string
        addrInfo.mReqCmd = mHttpUtils->GenerateRequest(urlInfo.mUrl,
            urlInfo.mUrlParams, aMethod);
        addrInfo.mAddr = urlInfo.mIpAddress;
        addrInfo.mPort = urlInfo.mPort;
        addrInfo.mProtocol = TCP;
        // Request
        string response;
        if (!SimpleHttpRequ(&addrInfo, response, aTimeout)) {
            return false;
        }

        // parse response
        if (response.empty()) {
            mErrCode = kErrRespNull;
            return false;
        }

        mHttpUtils->ParseResponse(response);
        return true;
    }

    bool HttpClient::OnlyPost(const string& aUrl, const string &aBody, int &aSockId) {
        struct SUrlInfo urlInfo;
        if (!UrlInfo(aUrl, &urlInfo)) {
            return false;
        }
        int tryTimes = 2;
        do {
            mErrCode = kOkHttpOK;
            if (aSockId < 2) {
                SAddrInfo addrInfo;
                addrInfo.mAddr = urlInfo.mIpAddress;
                addrInfo.mPort = urlInfo.mPort;
                addrInfo.mProtocol = TCP;
                if (RET_ERROR == mSocket->MakeTcpConn(&addrInfo)) {
                    mErrCode = kErrConnServer;
                    break;
                }
                aSockId = addrInfo.mSockId;
            }

            string header;
            StrUtils::Format(header,
                "POST %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Content-Type: application/json\r\n"
                "User-Agent: Mozilla/4.0 (compatible; KD-Exchange)\r\n"
                "Cache-Control: no-cache\r\n"
                "Connection: Keep-Alive\r\n"
                "Content-Length: %u\r\n\r\n",
                urlInfo.mUrl.c_str(), urlInfo.mHost.c_str(), aBody.length());
            INIT_IOV(2);
            SET_IOV_LEN(header.c_str(), header.length());
            SET_IOV_LEN(aBody.c_str(), aBody.length());
            if (RET_ERROR == mSocket->TcpSendV(aSockId, iovs, 2, 2, true)) {
                mErrCode = kErrSendToServer;
                mSocket->CloseSocket(aSockId);
                aSockId = -1;
            }
        } while(mErrCode != kOkHttpOK && --tryTimes > 0);
        return mErrCode == kOkHttpOK;
    }

    bool HttpClient::OnlyGet(const string& aUrl, int& aSockId, int aTimeout) {
        struct SUrlInfo urlInfo;
        if (!UrlInfo(aUrl, &urlInfo)) {
            return false;
        }
        int tryTimes = 2;
        do {
            mErrCode = kOkHttpOK;
            if (aSockId < 2) {
                SAddrInfo addrInfo;
                addrInfo.mAddr = urlInfo.mIpAddress;
                addrInfo.mPort = urlInfo.mPort;
                addrInfo.mProtocol = TCP;
                if (RET_ERROR == mSocket->MakeTcpConn(&addrInfo)) {
                    mErrCode = kErrConnServer;
                    break;
                }
                aSockId = addrInfo.mSockId;
            }
            string urlParams = urlInfo.mUrl;
            if (!urlInfo.mUrlParams.empty()) {
                urlParams += "?" + urlInfo.mUrlParams;
            }
            string header;
            StrUtils::Format(header,
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Content-Type: application/json\r\n"
                "User-Agent: Mozilla/4.0 (compatible; KD-Exchange)\r\n"
                "Cache-Control: no-cache\r\n"
                "Connection: Keep-Alive\r\n\r\n",
                urlParams.c_str(), urlInfo.mHost.c_str());
            if (RET_ERROR == mSocket->TcpSend(aSockId, header.c_str(),
                header.length(), aTimeout, true)) {
                mErrCode = kErrSendToServer;
                mSocket->CloseSocket(aSockId);
                aSockId = -1;
            }
        } while(mErrCode != kOkHttpOK && --tryTimes > 0);
        return mErrCode == kOkHttpOK;
    }

    bool HttpClient::Post(const string& aUrl, const string &aInBody,
        int &aSockId, string& aOutBody, int aTimeout) {
        if (OnlyPost(aUrl, aInBody, aSockId)) {
            return RecvResp(aSockId, aOutBody, aTimeout);            
        }
        return false;
    }

    bool HttpClient::Get(const string& aUrl, int &aSockId,
        string& aOutBody, int aTimeout) {        
        if (OnlyGet(aUrl, aSockId, aTimeout)) {
            return RecvResp(aSockId, aOutBody, aTimeout);            
        }
        return false;
    }

    bool HttpClient::RecvResp(int& aSockId, string& aOutBody, int aTimeout) {
        mErrCode = kOkHttpOK;
        char recvBuf[MAX_SIZE_4K+1] = {0};
        if (RET_ERROR == mSocket->RecvHttpPkg(aSockId, recvBuf,
            MAX_SIZE_4K, aTimeout, false)) {
            mErrCode = kErrRespNull;
            mSocket->CloseSocket(aSockId);
            aSockId = -1;
        } else {
            const char* bodyPtr = HttpUtils::GetBodyStr(recvBuf);
            if (bodyPtr != NULL) aOutBody = bodyPtr;
        }
        return mErrCode == kOkHttpOK;
    }

    // URL �Ƿ���Ч
    // \param url HTTP����URL
    // \param server ������IP,Ϊ���ַ�������ݲ���1���,Ĭ��Ϊ���ַ���,
    // ������url,server����������������ַ��Ϣ,��������ʧ��
    // \param port �������˿�,Ĭ��Ϊ80
    // \retval true URL��Ч
    // \retval false URL��ʧЧ
    bool HttpClient::IsUrlExist(const string& aUrl) {
        bool res = false;

        // check
        if (Request(aUrl, "HEAD", 5) && Done())
            res = true;
        mHttpUtils->Clear();

        return res;
    }

    // ִ��HTTP�����Ƿ�ɹ�
    // \retval true �ɹ�
    // \retval false ʧ��
    bool HttpClient::Done() const {
        string httpStatus = mHttpUtils->Status();
        int ret = stoi(httpStatus);
        if (ret>=100 && ret<300)
            return true;
        return false;
    }

    string HttpClient::ErrDesc() const {
        switch(mErrCode) {
            case kOkHttpOK:
                return "HTTP OK.";
            case kErrCreateSocket:
                return "socket error.";
            case kErrConnServer:
                return "can not connect to server.";
            case kErrSendToServer:
                return "send request failed to server.";
            case kErrRespTimeout:
                return "response time out.";
            case kErrBadHttpAddress:
                return "bad http server address.";
            case kErrRequNull:
                return "http request is null.";
            case kErrRespNull:
                return "http response is null.";
            case kErrInvalidResponse:
                return "invalid response.";
            case kErrRespStatus:
                return "bad http response status.";
            case kErrUnknown:
                return "Unknow Error.";
        }
        return "Unknow Error.";
    }
} // namespace common
