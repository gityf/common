/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class HttpUtils.
*/
#include <cstdlib>
#include <cstring>
#include <set>
#include <map>
#include <iostream>
#include <vector>
#include "base/stringutils.h"
#include "base/base64.h"
#include "httputils.h"

namespace common {
    static char sChar2EscapeTmp[] = {
    0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
    0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
    0x1C, 0x1D, 0x1E, 0x1F, '"', '#', '%', '\'', '*',
    '/', ':', '=', '?', '\\', 0x7F, '{', '[', ']'};
    //*/
    static std::set<char> sChar2Escape(sChar2EscapeTmp, sChar2EscapeTmp + sizeof(sChar2EscapeTmp));
    // http encode.
    std::string HttpUtils::EecodeUri(const std::string& srcStr) {
        std::string ret;
        for (size_t i = 0; i < srcStr.size(); ++i) {
            const char& c = srcStr[i];
            if (c >= 0 && c < 0x7F && (sChar2Escape.find(c) != sChar2Escape.end()))
            {
                ret.append("%" + StrUtils::Hex(c));
            } else {
                ret.append(&c, 1);
            }
        }
        return ret;
    }

    // http decode.
    int HttpUtils::DecodeUri(char* to, const char* from) {
        char *toPtr = to;
        for (; *from != '\0'; from++) {
            if (from[0] == '+') {
                *to++ = ' ';
            } else if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
                if (from[1] == '0' && (from[2] == 'D' || from[2] == 'd')) {
                    *to = 0x0d;
                } else if(from[1] == '0' && (from[2] == 'A' || from[2] == 'a')) {
                    *to = 0x0a;
                } else {
                    *to = HexIt(from[1]) * 16 + HexIt(from[2]);
                }
                to++;
                from += 2;
            } else {
                *to++ = *from;
            }
        }
        *to = '\0';
        return to - toPtr;
    }

    // http hexadecimal decodes.
    int HttpUtils::HexIt(char c) {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return 0;
    }

    // param name Header name and value
    void HttpUtils::SetHeader(const string &name, const string &value) {
        if (!name.empty()) {
            mSetMap[name] = value;
        }
    }

    // param referer Referer Header value
    void HttpUtils::SetReferer(const string &referer) {
        if (referer.empty()) {
            SetHeader("Referer", referer);
        }
    }

    // set HTTP request Authorization Header
    void HttpUtils::SetBasicAuth(const string &username, const string &password) {
        if (!username.empty()) {
            string auth = username + ":" + password;
            string authOut;
            Base64Encode(auth, &authOut);
            auth = "Basic " + authOut;
            SetHeader("Authorization", auth);
        }
    }

    // set HTTP request Proxy Authorization Header
    void HttpUtils::SetProxyAuth(const string &username, const string &password) {
        if (!username.empty()) {
            string auth = username + ":" + password;
            string authOut;
            Base64Encode(auth, &authOut);
            auth = "Basic " + authOut;
            SetHeader("Proxy-Authorization", auth);
        }
    }

    // set HTTP request Cookie Header
    void HttpUtils::SetCookie(const string &name, const string &value) {
        if (!name.empty()) {
            if (mSetMap.find("Cookie") != mSetMap.end()) {
                mSetMap["Cookie"] += "; ";
            }
            mSetMap["Cookie"] += (EecodeUri(name) + "=" + EecodeUri(value));
        }
    }

    // set HTTP request CGI param.
    void HttpUtils::SetUrlParam(const string &name, const string &value) {
        if (!name.empty()) {
            if (mUrlParams != "") mUrlParams += "&";
            mUrlParams += (EecodeUri(name) + "=" + EecodeUri(value));
        }
    }

    // Parse HTTP URL from urlstr to aUrlInfo
    bool HttpUtils::ParseUrl(const string &urlstr, struct SUrlInfo &aUrlInfo) {
        string url;
        StrUtils::TrimWhitespace(urlstr, kTrimAll, &url);
        if (url.empty()) {
            return false;
        }

        // parse hostname and url
        size_t pos;
        aUrlInfo.mHost = "";
        aUrlInfo.mUrl = url;
        if (url.compare(0, 7, "http://") == 0) {
            // http://...
            if ((pos=url.find("/",7)) != url.npos) {
                // http://hostname/...
                aUrlInfo.mHost = url.substr(7, pos-7);
                aUrlInfo.mUrl = url.substr(pos);
            } else {
                // http://hostname
                aUrlInfo.mHost = url.substr(7);
                aUrlInfo.mUrl = "/";
            }
        }

        // parse param
        aUrlInfo.mUrlParams = "";
        if ((pos=aUrlInfo.mUrl.rfind("?")) != aUrlInfo.mUrl.npos) {
            // cgi?param
            aUrlInfo.mUrlParams = aUrlInfo.mUrl.substr(pos+1);
            aUrlInfo.mUrl.erase(pos);
        }

        // parse port
        aUrlInfo.mPort = 80;
        if ((pos=aUrlInfo.mHost.rfind(":")) != aUrlInfo.mHost.npos) {
            // hostname:post
            aUrlInfo.mPort = atoi(aUrlInfo.mHost.substr(pos+1).c_str());
            aUrlInfo.mHost.erase(pos);
        }
        return true;
    }

    // pp=xx&qq=yyyy&rr=Zzzzz
    // to map[pp] = xx; map[qq] = yyyy; map[rr] = Zzzzz;
    bool HttpUtils::ParseParams(const string& aParams, std::map<string, string>& aOutPair) {
        std::vector<string> pairs;
        StrUtils::SplitString(aParams, "&", &pairs);
        if (pairs.empty()) {
            return false;
        }
        
        for (int ii = 0; ii < pairs.size(); ++ii) {
            string key, value;
            StrUtils::Split(pairs[ii], '=', key, value);
            aOutPair.insert(std::make_pair(key, value));
        }
        return true;
    }

    // Agent: AgentDetailDesc\r\n
    // Content-Type: application/json; charset=utf-8\r\n
    // to map[Agent] = AgentDetailDesc; map[Content-Type] = application/json;
    bool HttpUtils::ParseHeaders(const string& aHeader, std::map<string, string>& aOutPair) {
        std::vector<string> pairs;
        StrUtils::SplitString(aHeader, HTTP_CRLF, &pairs);
        if (pairs.empty()) {
            return false;
        }

        for (int ii = 0; ii < pairs.size(); ++ii) {
            string key, value;
            StrUtils::Split(pairs[ii], ':', key, value);
            aOutPair.insert(std::make_pair(key, value));
        }
        return true;
    }

    // Parse HTTP GET.POST URL
    bool HttpUtils::ParseRequest(const string &aRequ, string& aOutUrl, string& aParams) {
        string url;
        StrUtils::TrimWhitespace(aRequ, kTrimAll, &url);
        if (url.empty()) {
            return false;
        }
        do {
            size_t pos;
            if ((pos = url.rfind(" HTTP/")) != url.npos) {
                url.erase(pos);
            } else {
                break;
            }
            if ((pos = url.find(" /")) != url.npos) {
                url.erase(0, pos+2);
            } else {
                break;
            }
            if ((pos=url.find("?")) != url.npos) {
                // cgi?param
                aParams = url.substr(pos+1);
                url.erase(pos);
            }
            if (!url.empty()) {
                aOutUrl = url;
            }
            return true;
        } while (0);
        return false;
    }

    // generate HTTP request string.
    string HttpUtils::GenerateRequest(const string& aUrl,
        const string& aParams, const string& aMethod) {
        string Request;
        Request.reserve(512);

        Request += aMethod + " " + aUrl;
        if (aMethod != "POST" && aParams != "")
            Request += "?" + aParams;
        Request += " HTTP/1.1" + HTTP_CRLF;
        Request += "Accept: */*" + HTTP_CRLF;
        Request += "User-Agent: Mozilla/5.0 (compatible; HttpUtils)" + HTTP_CRLF;
        Request += "Pragma: no-cache" + HTTP_CRLF;
        Request += "Cache-Control: no-cache" + HTTP_CRLF;

        map<string,string>::const_iterator i;
        for (i = mSetMap.begin(); i != mSetMap.end(); ++i) {
            if (!i->first.empty()) {
                Request += i->first + ": " + i->second + HTTP_CRLF;
            }
        }

        Request += "Connection: Keep-Alive" + HTTP_CRLF;

        if (aMethod == "POST") {
            // post data
            Request += "Content-Length: " + to_string(aParams.length()) + HTTP_CRLF;
            Request += HTTP_CRLF;
            Request += aParams;
        } else {
            // get data
            Request += HTTP_CRLF;
        }
        return Request;
    }

    // parse HTTP response body.
    bool HttpUtils::ParseResponse(const string &response) {
        // Clear response Status
        mHttpStatus = "";
        mHttpBody = "";
        mGetMap.clear();

        // split header and body
        size_t pos;
        string head;
        string body;
        if ((pos=response.find(DOUBLE_CRLF)) != response.npos) {
            head = response.substr(0, pos);
            body = response.substr(pos+4);
        } else if ((pos=response.find("\n\n")) != response.npos) {
            head = response.substr(0, pos);
            body = response.substr(pos+2);
        } else {
            return false;
        }

        // parse Status
        string status;
        if ((pos=head.find(HTTP_CRLF)) != head.npos) {
            status = head.substr(0, pos);
            head.erase(0, pos+2);

            // HTTP/1.1 status_number description_string
            mGetMap["HTTP_STATUS"] = status;
            if (status.compare(0, 5, "HTTP/") == 0) {
                size_t b1, b2;
                if ((b1=status.find(" "))!=status.npos
                    && (b2=status.find(" ",b1+1))!=status.npos)
                    mHttpStatus = status.substr(b1+1, b2-b1-1);
            }
        }

        // parse header
        string line, name, value;
        vector<string> headList;
        StrUtils::SplitString(head, "\n", &headList);
        int ii = 0;
        for (ii = 0; ii < headList.size(); ++ii) {
            line = headList[ii];
            StrUtils::TrimWhitespace(line, kTrimAll, &line);
            // name: value
            if ((pos=line.find(":")) != line.npos) {
                name = line.substr(0, pos);
                StrUtils::TrimWhitespace(name, kTrimAll, &name);
                value = line.substr(pos+1);
                StrUtils::TrimWhitespace(value, kTrimAll, &value);

                if (name != "") {
                    if (mGetMap[name] != "")
                        mGetMap[name] += "\n";
                    mGetMap[name] += value;
                }
            }
        }

        // parse body
        if (this->GetHeader("Transfer-Encoding") == "chunked")
            mHttpBody = this->ParseChunked(body);
        else
            mHttpBody = body;
    }

    // get http header by name.
    string HttpUtils::GetHeader(const string &name) {
        if (!name.empty()) {
            return mGetMap[name];
        } else {
            return string("");
        }
    }

    // get HTTP response Set-Cookie Header
    void HttpUtils::GetCookie(std::vector<std::string> *r) {
        string ck = this->GetHeader("Set-Cookie");
        StrUtils::SplitString(ck, "\n", r);
    }

    // get HTTP header message.
    string HttpUtils::DumpHeader() {
        // Status
        string header = (this->GetHeader("HTTP_STATUS") + "\n");

        // for multi header
        string headers;
        vector<string> headerlist;

        map<string,string>::const_iterator iter;
        for (iter = mGetMap.begin(); iter != mGetMap.end(); ++iter) {
            if (!iter->first.empty() && iter->first != "HTTP_STATUS") {
                if ((iter->second).find("\n") != (iter->second).npos) {
                    headers = iter->second;
                    StrUtils::SplitString(headers, "\n", &headerlist);
                    for (size_t j=0; j < headerlist.size(); ++j)
                        header += iter->first + ": " + headerlist[j] + "\n";
                } else {
                    header += iter->first + ": " + iter->second + "\n";
                }
            }
        }

        return header;
    }

    // clear all request and response HTTP header and body.
    void HttpUtils::Clear() {
        // set
        mUrlParams = "";
        mSetMap.clear();

        // get
        mHttpStatus = "";
        mHttpBody = "";
        mGetMap.clear();
    }

    // parse HTTP body content type of chunked.
    string HttpUtils::ParseChunked(const string &chunkedstr) {
        char crlf[3] = "\x0D\x0A";
        size_t pos, lastpos;
        long size = 0;
        string hexstr;

        // location HTTP_CRLF
        if ((pos=chunkedstr.find(crlf)) != chunkedstr.npos) {
            hexstr = chunkedstr.substr(0, pos);
            size = StrUtils::stol(hexstr, 16);
        }

        string res;
        res.reserve(chunkedstr.length());

        while (size > 0) {
            // append to Body
            res += chunkedstr.substr(pos+2, size);
            lastpos = pos+size+4;

            // location next HTTP_CRLF
            if ((pos=chunkedstr.find(crlf,lastpos)) != chunkedstr.npos) {
                hexstr = chunkedstr.substr(lastpos, pos-lastpos);
                size = StrUtils::stol(hexstr, 16);
            } else {
                break;
            }
        }

        return res;
    }

    // header = "Content-Length: 123\r\n", type = "Content-Length"
    // return 123\r\n
    string HttpUtils::GetHeaderItem(const char* aHeader, const char* aType) {
        const char* p = strcasestr(aHeader, aType);
        if (p == NULL) {
            return "";
        }
        p += (strlen(aType) + 1);
        while (isblank(*p) != 0) {
            p++;
        }
        const char* pEnd = strstr(p, HTTP_CRLF.c_str());
        if (pEnd != NULL) {
            string retstr;
            retstr.assign(p, pEnd);
            return retstr;
        } else {
            return p;
        }
    }

    // header = "Content-Length: 123", type = "Content-Length"
    int HttpUtils::GetHeaderValue(const char* aHeader, const char* aType) {
        string p = GetHeaderItem(aHeader, aType);
        return (p.empty()) ? -1 : atoi(p.c_str());
    }

    // HTTP body
    const char* HttpUtils::GetBodyStr(const char* aData) {
        const char* p = strstr(aData, DOUBLE_CRLF.c_str());
        return (p != NULL) ? p + 4 : NULL;
    }
} // namespace common
