#include "base/randomizer.h"
#include "ut/test_harness.h"

TEST(TestRandomizer, TestRand)
{
    common::Randomizer r1;
    int a = r1.Rand(1000);
    ASSERT_LT(a, 1000);
    common::Randomizer r2(r1.GetSeed());
    ASSERT_EQ(r1.GetSeed(), r2.GetSeed());

    int b = r2.Rand(1000);
    ASSERT_LT(b, 1000);

    r2.SetSeed(r1.GetSeed() + 1);
    ASSERT_EQ(r1.GetSeed() + 1, r2.GetSeed());

    b = r2.Rand(1000);
    ASSERT_LT(b, 1000);
}

