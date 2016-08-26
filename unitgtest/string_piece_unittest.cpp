// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/03/2015
// Description: file of string for char test.


#include "base/string_piece.h"

#include "ut/test_harness.h"

using namespace std;
using namespace common;

TEST(StringPieceTest, BasicTest)
{
    std::string s1("abcd");
    StringPiece sp1(s1);
    EXPECT_EQ(sp1[2], 'c');
    EXPECT_TRUE(!sp1.empty());

    std::string s2("efghi");
    sp1.set(s2);
    EXPECT_EQ(sp1[2], 'g');

    sp1.clear();
    EXPECT_EQ(sp1.length(), 0U);
    EXPECT_TRUE(sp1.empty());

    StringPiece sp2(s1);
    EXPECT_LT(sp1.compare(sp2), 0);

    std::string s3;
    for (StringPiece::const_iterator it = sp2.begin(); it != sp2.end(); ++it) {
        s3 += *it;
    }
    EXPECT_TRUE(s1 == s3);

    std::string s4;
    for (StringPiece::const_reverse_iterator it = sp2.rbegin();
         it != sp2.rend();
         ++it) {
        s4 += *it;
    }
    sp2.set(s4);
    std::string s5;
    for (StringPiece::const_reverse_iterator it = sp2.rbegin();
         it != sp2.rend();
         ++it) {
        s5 += *it;
    }
    EXPECT_TRUE(s1 == s5);
}

TEST(StringPieceTest, AssignTest)
{
    std::string s1("abcd");
    StringPiece sp1(s1);
    StringPiece sp2(sp1);
    sp2 = sp1;
    sp2 = s1;

    EXPECT_TRUE(sp2 == sp1);
}

TEST(StringPieceTest, FindTest)
{
    std::string s1("abcd1234abcd");
    StringPiece sp1(s1);
    std::string s2("abc");
    StringPiece sp2(s2);

    size_t pos_sp = 0;
    size_t pos_str  = 0;

    pos_sp = sp1.find(sp2, pos_sp);
    pos_str = s1.find(s2, pos_str);

    EXPECT_EQ(pos_sp, pos_str);

    EXPECT_EQ(sp1.find('1'), 4U);

    const char* nullPtr = NULL;
    EXPECT_EQ(sp1.find(nullPtr), 0U);

    // the following line is workaround for gtest bug
    size_t npos = StringPiece::npos;
    EXPECT_EQ(sp1.find(sp2, npos), npos);
}

TEST(StringPieceTest, RfindTest)
{
    std::string s1("abcd1234abcd");
    StringPiece sp1(s1);
    std::string s2("abc");
    StringPiece sp2(s2);

    size_t pos_sp = -1;
    size_t pos_str = -1;

    pos_sp = sp1.rfind(sp2, pos_sp);
    pos_str = s1.rfind(s2, pos_str);

    EXPECT_EQ(pos_sp, pos_str);

    EXPECT_EQ(sp1.rfind('1'), 4U);

    const char* nullPtr = NULL;
    EXPECT_EQ(sp2.rfind(nullPtr), 3U);
}

TEST(StringPieceTest, FindFirstOfTest)
{
    std::string s1("abcd1234abcd");
    StringPiece sp1(s1);
    std::string s2("0123456789");
    StringPiece sp2(s2);

    size_t pos_sp = sp1.find_first_of(sp2);
    size_t pos_str = s1.find_first_of(s2);

    EXPECT_EQ(pos_sp, pos_str);

    EXPECT_EQ(sp1.find_first_of('1'), 4U);
}

TEST(StringPieceTest, FindLastOfTest)
{
    std::string s1("abcd1234abcd");
    StringPiece sp1(s1);
    std::string s2("0123456789");
    StringPiece sp2(s2);

    size_t pos_sp = sp1.find_last_of(sp2);
    size_t pos_str = s1.find_last_of(s2);

    EXPECT_EQ(pos_sp, pos_str);

    EXPECT_EQ(sp1.find_last_of('1'), 4U);
}

TEST(StringPieceTest, FindFirstNotOfTest)
{
    std::string s1("abcd1234abcd");
    StringPiece sp1(s1);
    std::string s2("abcd");
    StringPiece sp2(s2);

    size_t pos_sp = sp1.find_first_not_of(sp2);
    size_t pos_str = s1.find_first_not_of(s2);

    EXPECT_EQ(pos_sp, pos_str);

    EXPECT_EQ(sp1.find_first_not_of('1'), 0U);
}

TEST(StringPieceTest, FindLastNotOfTest)
{
    std::string s1("abcd1234abcd");
    StringPiece sp1(s1);
    std::string s2("abcd");
    StringPiece sp2(s2);

    size_t pos_sp = sp1.find_last_not_of(sp2);
    size_t pos_str = s1.find_last_not_of(s2);

    EXPECT_EQ(pos_sp, pos_str);

    EXPECT_EQ(sp1.find_last_not_of('1'), 11U);
}

TEST(StringPieceTest, SubStrTest)
{
    std::string s1("abcd1234abcd");
    StringPiece sp(s1);
    size_t len = sp.size();

    StringPiece s = sp.substr();
    EXPECT_EQ(s.size(), len);

    s = sp.substr(3);
    EXPECT_EQ(s.size(), len - 3);

    s = sp.substr(3, 5);
    EXPECT_EQ(s.size(), 5U);
}

TEST(StringPieceTest, CharAccessTest)
{
    std::string s1("abcd1234abcd");
    StringPiece sp(s1);

    EXPECT_EQ(sp.at(0), 'a');
    EXPECT_EQ(sp[0], 'a');
}

TEST(StringPieceTest, CompareTest)
{
    StringPiece sp1;
    StringPiece sp2;
    EXPECT_TRUE(sp1 == sp2);

    const char* s = "abcd1234abcd";
    sp1.set(s, 4);
    sp2.set(s + 8, 4);
    EXPECT_TRUE(sp1 == sp2);
}
