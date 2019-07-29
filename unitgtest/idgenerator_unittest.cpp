#include "../id/snowflakeid.hpp"
#include "../id/idgenerator.hpp"
#include "ut/test_harness.h"
#include "base/timemeasurer.h"
#include <iostream>

TEST(SnowFlakeUnitTest, Test1)
{
    SnowFlakeId snowFlakeId;
    snowFlakeId.setDatacenterId(22);
    snowFlakeId.setWorkerId(11);

    uint64_t id;
    EXPECT_EQ(snowFlakeId.nextId(id), true);
    std::cout << id << std::endl;
    snowFlakeId.nextId(id);
    std::cout << id << std::endl;
    TimeMeasurer tm;
    const size_t count = 20000000;
    for (size_t i = 0; i < count; i++)
    {
        snowFlakeId.nextId(id);
    }
    std::cout << "cost(ms):" << tm.Elapsed() << std::endl;
}

TEST(IdGenerateUnitTest, Test1)
{
    IdGenerator idGenerator;
    idGenerator.setMachineId(11);
    idGenerator.init();


    uint64_t id;
    EXPECT_EQ(idGenerator.nextId(id), true);
    std::cout << id << std::endl;
    idGenerator.nextId(id);
    std::cout << id << std::endl;
    TimeMeasurer tm;
    const size_t count = 20000000;
    for (size_t i = 0; i < count; i++)
    {
        idGenerator.nextId(id);
    }
    std::cout << "cost(ms):" << tm.Elapsed() << std::endl;
}

