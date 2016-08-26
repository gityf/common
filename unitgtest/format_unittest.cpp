/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of Thread ut.
*/
#include <iostream>
#include "base/format.h"
#include "ut/test_harness.h"
using namespace std;

TEST(StrFormatTest, StringFormatFuncTest)
{
    string str = "str=123";
    string formatStr = common::format("str=%d", 123);
    EXPECT_EQ(formatStr, str);
    formatStr = common::format("%s,%d,%u,%f,%04x", "str", 100, 300, 11.22f, 65535);
    EXPECT_EQ(formatStr, "str,100,300,11.220000,ffff");
}
