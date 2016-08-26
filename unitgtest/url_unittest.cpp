/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of Thread ut.
*/
#include <iostream>
#include <string>
#include <vector>
#include "net/http/url.h"
#include "ut/test_harness.h"
using namespace std;
TEST(Uri_test, testUri_UPHP) {
    common::net::Uri uri("http://user:password@host:56/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "user");
    EXPECT_EQ(uri.password(), "password");
    EXPECT_EQ(uri.host(), "host");
    EXPECT_EQ(uri.port(), 56);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_UHP) {
    common::net::Uri uri("http://user@host:56/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "user");
    EXPECT_EQ(uri.password(), "");
    EXPECT_EQ(uri.host(), "host");
    EXPECT_EQ(uri.port(), 56);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_UPH) {
    common::net::Uri uri("http://user:password@host/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "user");
    EXPECT_EQ(uri.password(), "password");
    EXPECT_EQ(uri.host(), "host");
    EXPECT_EQ(uri.port(), 80);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_HP) {
    common::net::Uri uri("http://host:56/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "");
    EXPECT_EQ(uri.password(), "");
    EXPECT_EQ(uri.host(), "host");
    EXPECT_EQ(uri.port(), 56);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_H) {
    common::net::Uri uri("http://host/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "");
    EXPECT_EQ(uri.password(), "");
    EXPECT_EQ(uri.host(), "host");
    EXPECT_EQ(uri.port(), 80);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_UPH6P) {
    common::net::Uri uri("http://user:password@[::1]:56/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "user");
    EXPECT_EQ(uri.password(), "password");
    EXPECT_EQ(uri.host(), "::1");
    EXPECT_EQ(uri.port(), 56);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_UH6P) {
    common::net::Uri uri("http://user@[::1]:56/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "user");
    EXPECT_EQ(uri.password(), "");
    EXPECT_EQ(uri.host(), "::1");
    EXPECT_EQ(uri.port(), 56);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_UPH6) {
    common::net::Uri uri("http://user:password@[::1]/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "user");
    EXPECT_EQ(uri.password(), "password");
    EXPECT_EQ(uri.host(), "::1");
    EXPECT_EQ(uri.port(), 80);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_H6P) {
    common::net::Uri uri("http://[::1]:56/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "");
    EXPECT_EQ(uri.password(), "");
    EXPECT_EQ(uri.host(), "::1");
    EXPECT_EQ(uri.port(), 56);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testUri_H6) {
    common::net::Uri uri("http://[::1]/blah.html");

    EXPECT_EQ(uri.protocol(), "http");
    EXPECT_EQ(uri.user(), "");
    EXPECT_EQ(uri.password(), "");
    EXPECT_EQ(uri.host(), "::1");
    EXPECT_EQ(uri.port(), 80);
    EXPECT_EQ(uri.path(), "/blah.html");
}

TEST(Uri_test, testQuery) {
    common::net::Uri uri("http://host/?abc=1");
    EXPECT_EQ(uri.query(), "abc=1");
}

TEST(Uri_test, testFragment) {
    common::net::Uri uri("http://host/#foo");
    EXPECT_EQ(uri.fragment(), "foo");
}

TEST(Uri_test, testQueryFragment) {
    common::net::Uri uri("http://host/?abc=1#foo");
    EXPECT_EQ(uri.query(), "abc=1");
    EXPECT_EQ(uri.fragment(), "foo");
}

TEST(Uri_test, testHttpPort) {
    common::net::Uri uri("http://host/");
    EXPECT_EQ(uri.port(), 80);
}

TEST(Uri_test, testHttpsPort) {
    common::net::Uri uri("https://host/");
    EXPECT_EQ(uri.port(), 443);
}

TEST(Uri_test, testFtpPort) {
    common::net::Uri uri("ftp://host/");
    EXPECT_EQ(uri.port(), 21);
}

TEST(Uri_test, testUriStr) {
    common::net::Uri uri("http://user:password@host:80/blah.html");
    EXPECT_EQ(uri.str(), "http://user:password@host/blah.html");
}