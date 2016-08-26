/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class HttpUtils.
*/
#ifndef _COMMON_CHTTPUTILS_H_
#define _COMMON_CHTTPUTILS_H_

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace common {

    const string HTTP_CRLF = "\r\n";
    const string DOUBLE_CRLF = "\r\n\r\n";

    struct SUrlInfo {
        string mHost;
        string mIpAddress;
        string mUrl;
        string mUrlParams;
        int mPort;
    };
    class HttpUtils {
    public:
        // Constructor
        HttpUtils(){};

        // Destructor
        virtual ~HttpUtils(){};

        // set HTTP header
        void SetHeader(const string &name, const string &value);
        // set HTTP request Referer Header
        void SetReferer(const string &referer);
        // set HTTP request Authorization Header
        void SetBasicAuth(const string &username, const string &password);
        // set HTTP request Proxy Authorization Header
        void SetProxyAuth(const string &username, const string &password);
        // set HTTP request Cookie Header
        void SetCookie(const string &name, const string &value);
        // set HTTP request CGI params
        void SetUrlParam(const string &name, const string &value);

        // get http header by name.
        string GetHeader(const string &name);
        // get HTTP Set-Cookie Header
        void GetCookie(std::vector<std::string> *r);
        string GetUrlParam() {
            return mUrlParams;
        }
        // dump all http message to string.
        string DumpHeader();
        // clear all request and response HTTP header and body.
        void Clear();

        // get HTTP response Status
        inline string Status() const {
            return mHttpStatus;
        }
        // get HTTP response Content.
        inline string Body() const {
            return mHttpBody;
        }
        // get HTTP response length of Content.(Content-Length)
        inline size_t ContentLength() const {
            return mHttpBody.length();
        }

        // HTTP request string.
        inline string RequestPack() const {
            return mRequest;
        }
        // HTTP response string.
        inline string ResponsePack() const {
            return mResonse;
        }
        // Parse HTTP URL string.
        bool ParseUrl(const string &urlstr, struct SUrlInfo &aUrlInfo);
        // fr=1&uid=xxxxxx&token=yyyyy&pl=ios
        // to fr:1, uid:xxxxxx, token:yyyyy, pl:ios
        bool ParseParams(const string& aParams, std::map<string, string>& aOutPair);
        // Agent: AgentDetailDesc\r\n
        // Content-Type: application/json; charset=utf-8\r\n
        // to map[Agent] = AgentDetailDesc; map[Content-Type] = application/json;
        bool ParseHeaders(const string& aHeader, std::map<string, string>& aOutPair);
        // Parse HTTP GET.POST URL
        bool ParseRequest(const string &aRequ, string& aOutUrl, string& aParams);
        // generate HTTP request string.
        string GenerateRequest(const string& aUrl, const string& aParams, const string& aMethod);
        // parse HTTP response
        bool ParseResponse(const string &response);
        // parse HTTP body content type of chunked.
        string ParseChunked(const string &chunkedstr);
        // http encode.
        std::string EecodeUri(const std::string& srcStr);

        // http decode.
        int DecodeUri(char* to, const char* from);

        // http hexadecimal decodes.
        int HexIt(char c);

        static string GetHeaderItem(const char* aHeader, const char* aType);
        static int GetHeaderValue(const char* aHeader, const char* aType);
        static const char* GetBodyStr(const char* aData);
    private:
        // set
        string mRequest;             // generated Request
        string mUrlParams;           // http Request params
        map<string, string> mSetMap; // push http headers

        // get
        string mResonse;             // server response
        string mHttpStatus;          // http response Status
        string mHttpBody;            // http response Body
        map<string, string> mGetMap; // recv http headers
    };

} // namespace

#endif // _COMMON_CHTTPUTILS_H_

