#include "base/circuitbreaker.hpp"
#include "ut/test_harness.h"

TEST(CBUnitTest, Test1)
{
    CircuitBreaker cb;
    EXPECT_EQ(cb.allow(), true);
}


