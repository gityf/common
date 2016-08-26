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
#include "base/cache.h"
#include "ut/test_harness.h"
using namespace std;

TEST(Cache_test, cacheTest) {
    common::Cache<int, int> cache(6);

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.put(4, 40);
    cache.put(5, 50);
    cache.put(6, 60);
    cache.put(7, 70);
    cache.put(8, 80);
    cache.put(9, 90);
    cache.put(10, 100);

    std::pair<bool, int> result;

    result = cache.getx(1);
    ASSERT_TRUE(result.first);
    EXPECT_EQ(result.second, 10);

    result = cache.getx(8);
    ASSERT_TRUE(result.first);
    EXPECT_EQ(result.second, 80);

    cache.put_top(11, 110);
    cache.put_top(12, 120);
    cache.put_top(13, 130);
    cache.put_top(14, 140);

    result = cache.getx(10);
    ASSERT_TRUE(!result.first);

    result = cache.getx(11);
    ASSERT_TRUE(result.first);
    EXPECT_EQ(result.second, 110);
}

TEST(Cache_test, erase) {
    common::Cache<int, int> cache(6);

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.put(4, 40);
    cache.put(5, 50);
    cache.put(6, 60);
    cache.put(7, 70);
    cache.put(8, 80);
    cache.put(9, 90);
    cache.put(10, 100);

    cache.erase(2);
    EXPECT_EQ(cache.size(), 5);
    EXPECT_EQ(cache.winners(), 3);

    cache.erase(9);
    EXPECT_EQ(cache.size(), 4);
    EXPECT_EQ(cache.winners(), 3);

}


TEST(Cache_test, resize) {
    common::Cache<int, int> cache(6);

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.put(4, 40);
    cache.put(5, 50);
    cache.put(6, 60);
    cache.put(7, 70);
    cache.put(8, 80);
    cache.put(9, 90);
    cache.put(10, 100);

    EXPECT_EQ(cache.size(), 6);
    EXPECT_EQ(cache.winners(), 3);

    cache.setMaxElements(8);

    EXPECT_EQ(cache.size(), 6);
    EXPECT_EQ(cache.winners(), 4);

    cache.setMaxElements(4);

    EXPECT_EQ(cache.size(), 4);
    EXPECT_EQ(cache.winners(), 2);

    cache.setMaxElements(8);

    EXPECT_EQ(cache.size(), 4);
    EXPECT_EQ(cache.winners(), 4);

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.put(4, 40);
    cache.put(5, 50);
    cache.put(6, 60);
    cache.put(7, 70);
    cache.put(8, 80);
    cache.put(9, 90);
    cache.put(10, 100);

    cache.setMaxElements(4);

    EXPECT_EQ(cache.size(), 4);
    EXPECT_EQ(cache.winners(), 2);

}

TEST(Cache_test, stats) {
    common::Cache<int, int> cache(6);

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.put(4, 40);
    cache.put(5, 50);

    cache.getx(1);
    cache.getx(2);
    cache.getx(8);

    EXPECT_EQ(cache.getHits(), 2);
    EXPECT_EQ(cache.getMisses(), 1);
}
