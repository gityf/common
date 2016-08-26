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
#include "base/arg.h"
#include "ut/test_harness.h"
using namespace std;
namespace {
    std::vector<char*> argp;

    char* arg(const char* value) {
        argp.push_back(new char[strlen(value) + 1]);
        strcpy(argp.back(), value);
        return argp.back();
    }
}
TEST(Arg_test, testArgBool) {
    int argc = 7;
    char* argv[] = { arg("prog"), arg("-i"), arg("-j"), arg("-k"),
        arg("-cf"), arg("--foo"), arg("foo"), 0 };

    common::Arg<bool> optionJ(argc, argv, 'j');
    common::Arg<bool> optionL(argc, argv, 'l');
    common::Arg<bool> optionC(argc, argv, 'c');
    common::Arg<bool> optionFoo(argc, argv, "--foo");
    common::Arg<bool> optionBar(argc, argv, "--bar");

    ASSERT_TRUE(optionJ);
    ASSERT_TRUE(!optionL);
    ASSERT_TRUE(optionC);
    ASSERT_TRUE(optionFoo);
    ASSERT_TRUE(!optionBar);
    EXPECT_EQ(argc, 5);
    EXPECT_EQ(strcmp(argv[0], "prog"), 0);
    EXPECT_EQ(strcmp(argv[1], "-i"), 0);
    EXPECT_EQ(strcmp(argv[2], "-k"), 0);
    EXPECT_EQ(strcmp(argv[3], "-f"), 0);
    EXPECT_EQ(strcmp(argv[4], "foo"), 0);
}

TEST(Arg_test, testArgCharP) {
    int argc = 7;
    char* argv[] = { arg("prog"), arg("-i"), arg("foo"), arg("--foo"),
        arg("inp"), arg("-k"), arg("bar"), 0 };

    common::Arg<const char*> optionI(argc, argv, 'i');
    common::Arg<const char*> optionL(argc, argv, 'l', "blah");
    common::Arg<const char*> optionFoo(argc, argv, "--foo");
    common::Arg<const char*> optionBar(argc, argv, "--bar");

    ASSERT_TRUE(optionI.isSet());
    ASSERT_TRUE(!optionL.isSet());
    ASSERT_TRUE(optionFoo.isSet());
    ASSERT_TRUE(!optionBar.isSet());
    EXPECT_EQ(strcmp(optionI.getValue(), "foo"), 0);
    EXPECT_EQ(strcmp(optionL.getValue(), "blah"), 0);
    EXPECT_EQ(strcmp(optionFoo.getValue(), "inp"), 0);
    EXPECT_EQ(argc, 3);
    EXPECT_EQ(strcmp(argv[0], "prog"), 0);
    EXPECT_EQ(strcmp(argv[1], "-k"), 0);
    EXPECT_EQ(strcmp(argv[2], "bar"), 0);
}

TEST(Arg_test, testArgpInt) {
    int argc = 6;
    char* argv[] = { arg("prog"), arg("-v"), arg("42"), arg("-i"), arg("-j5"), arg("--foo"), 0 };

    common::Arg<int> optionJ(argc, argv, 'j');
    common::Arg<int> optionV(argc, argv, 'v');

    ASSERT_TRUE(optionJ.isSet());
    ASSERT_TRUE(optionV.isSet());
    EXPECT_EQ(optionJ, 5);
    EXPECT_EQ(optionV, 42);
    EXPECT_EQ(argc, 3);
    EXPECT_EQ(strcmp(argv[0], "prog"), 0);
    EXPECT_EQ(strcmp(argv[1], "-i"), 0);
    EXPECT_EQ(strcmp(argv[2], "--foo"), 0);
}

TEST(Arg_test, testArgpCharp) {
    int argc = 7;
    char* argv[] = { arg("prog"), arg("-v"), arg("42"),
        arg("-I"), arg("include"), arg("-Jfoo"), arg("-Khello world"), 0 };

    common::Arg<const char*> optionI(argc, argv, 'I');
    common::Arg<const char*> optionJ(argc, argv, 'J');
    common::Arg<const char*> optionK(argc, argv, 'K');

    ASSERT_TRUE(optionI.isSet());
    ASSERT_TRUE(optionJ.isSet());
    ASSERT_TRUE(optionK.isSet());
    EXPECT_EQ(strcmp(optionI, "include"), 0);
    EXPECT_EQ(strcmp(optionJ, "foo"), 0);
    EXPECT_EQ(strcmp(optionK, "hello world"), 0);
    EXPECT_EQ(argc, 3);
    EXPECT_EQ(strcmp(argv[0], "prog"), 0);
    EXPECT_EQ(strcmp(argv[1], "-v"), 0);
    EXPECT_EQ(strcmp(argv[2], "42"), 0);
}

TEST(Arg_test, testArgpStdString) {
    int argc = 7;
    char* argv[] = { arg("prog"), arg("-v"), arg("42"), arg("-Jfoo"),
        arg("-I"), arg("include"), arg("-Khello world"), 0 };

    common::Arg<std::string> optionI(argc, argv, 'I');
    common::Arg<std::string> optionJ(argc, argv, 'J');
    common::Arg<std::string> optionK(argc, argv, 'K');

    ASSERT_TRUE(optionI.isSet());
    ASSERT_TRUE(optionJ.isSet());
    ASSERT_TRUE(optionK.isSet());
    EXPECT_EQ(optionI.getValue(), "include");
    EXPECT_EQ(optionJ.getValue(), "foo");
    EXPECT_EQ(optionK.getValue(), "hello world");
    EXPECT_EQ(argc, 3);
    EXPECT_EQ(strcmp(argv[0], "prog"), 0);
    EXPECT_EQ(strcmp(argv[1], "-v"), 0);
    EXPECT_EQ(strcmp(argv[2], "42"), 0);
}

