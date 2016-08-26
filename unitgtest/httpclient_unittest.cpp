
#include "base/singleton.h"
#include "net/httpclient.h"
#include "net/httputils.h"
#include "base/colorprint.h"
#include <iostream>
#include "ut/test_harness.h"
using namespace common;
TEST(CHCTestTEST, BasicTest)
{
    HttpClient* httpCln = new HttpClient();
    bool ret = httpCln->IsUrlExist("http://127.0.0.1:15218");
    if (ret) {
        ret = httpCln->Request("http://127.0.0.1:15218");
        EXPECT_EQ(true, ret);
        common::ColorGreen(cout) << "[ INFO     ] Header:\n" << httpCln->mHttpUtils->DumpHeader() << endl;
        common::ColorGreen(cout) << "[ INFO     ] Body:\n" << httpCln->mHttpUtils->Body() << endl;
        int fd = -1;
        string body;
        httpCln->Get("http://127.0.0.1:15218", fd, body);
        common::ColorGreen(cout) << "[ INFO     ] Body:\n" << body << endl;
    }
}

TEST(CHUTest, BasicTTT)
{
    string httpRespStr = "HTTP/1.1 200 OK\r\n"
                         "Date: Thu, 26 Feb 2015 01:53:49 GMT\r\n"
                         "Server: nginx/1.0.12\r\n"
                         "Content-Type: text/html\r\n"
                         "Accept-Ranges: bytes\r\n"
                         "Last-Modified: Tue, 18 Jun 2013 07:44:44 GMT\r\n"
                         "Content-Length: 10\r\n"
                         "Proxy-Connection: Keep-Alive\r\n\r\n1234567890";
    HttpUtils* httpUtils = new HttpUtils();
    httpUtils->ParseResponse(httpRespStr);

    EXPECT_EQ("1234567890", httpUtils->Body());
    EXPECT_EQ("200", httpUtils->Status());
    EXPECT_EQ("Thu, 26 Feb 2015 01:53:49 GMT", httpUtils->GetHeader("Date"));
    EXPECT_EQ("nginx/1.0.12", httpUtils->GetHeader("Server"));
    EXPECT_EQ("text/html", httpUtils->GetHeader("Content-Type"));
    EXPECT_EQ("bytes", httpUtils->GetHeader("Accept-Ranges"));
    EXPECT_EQ("Tue, 18 Jun 2013 07:44:44 GMT", httpUtils->GetHeader("Last-Modified"));
    EXPECT_EQ("10", httpUtils->GetHeader("Content-Length"));
    EXPECT_EQ("10", httpUtils->GetHeader("Content-Length"));
    EXPECT_EQ("Keep-Alive", httpUtils->GetHeader("Proxy-Connection"));

    SUrlInfo url;
    httpUtils->ParseUrl("http://127.0.0.1:15218/name?k=vv&k2=v2", url);
    EXPECT_EQ(url.mHost, "127.0.0.1");
    EXPECT_EQ(url.mPort, 15218);
    EXPECT_EQ(url.mUrl, "/name");
    EXPECT_EQ(url.mUrlParams, "k=vv&k2=v2");


    string getstr = "GET /index.html?ssid=123456&dd=ccccc HTTP/1.1\r\n";
    string poststr = "POST /index.html HTTP/1.1\r\n";
    string getUrl;
    string getParam;
    httpUtils->ParseRequest(getstr, getUrl, getParam);
    EXPECT_EQ(getUrl, "index.html");
    EXPECT_EQ(getParam, "ssid=123456&dd=ccccc");

    string postUrl;
    string postParam;
    httpUtils->ParseRequest(poststr, postUrl, postParam);
    EXPECT_EQ(postUrl, "index.html");
    EXPECT_EQ(postParam.empty(), true);
    delete httpUtils;
}
