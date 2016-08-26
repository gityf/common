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
#include "base/lrucache.h"
#include "ut/test_harness.h"
using namespace std;
TEST(LruCache_test, cacheTest) {
    common::LruCache<int, int> cache(6);

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
    ASSERT_TRUE(!result.first);

    result = cache.getx(8);
    ASSERT_TRUE(result.first);
    EXPECT_EQ(result.second, 80);

}

TEST(LruCache_test, erase) {
    common::LruCache<int, int> cache(6);

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

    cache.erase(2);
    EXPECT_EQ(cache.size(), 6);

    cache.erase(9);
    EXPECT_EQ(cache.size(), 5);

}


TEST(LruCache_test, resize) {
    common::LruCache<int, int> cache(6);

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

    cache.setMaxElements(8);

    EXPECT_EQ(cache.size(), 6);

    cache.setMaxElements(4);

    EXPECT_EQ(cache.size(), 4);

    cache.setMaxElements(8);

    EXPECT_EQ(cache.size(), 4);

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

}

TEST(LruCache_test, stats) {
    common::LruCache<int, int> cache(6);

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

