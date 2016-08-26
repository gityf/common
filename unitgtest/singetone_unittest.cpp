
#include "base/singleton.h"

#include <iostream>
#include "ut/test_harness.h"

class Foo
{
public:
    Foo() : mCount(0) {}
    ~Foo()
    {
        // following code is just to verify the dtor of the singleton object
        // has been called. disable it when checkin code.
        // std::cout << "Foo::~Foo()" << std::endl;
    }
    void Increment()
    {
        ++mCount;
    }
    void Decrement()
    {
        --mCount;
    }
    int Get()
    {
        return mCount;
    }
    void Reset()
    {
        mCount = 0;
    }

private:
    int mCount;
};

TEST(SingletonTest, BasicTest)
{
    {
        Foo& foo = Singleton<Foo>::InstanceRef();
        foo.Increment();
        foo.Increment();
    }
    // now verify the the singleton object from another scope
    {
        Foo& foo = Singleton<Foo>::InstanceRef();
        EXPECT_EQ(2, foo.Get());
    }
    {
        Foo *foo = Singleton<Foo>::Instance();
        foo->Increment();
        foo->Increment();
    }
    {
        Foo& foo = Singleton<Foo>::InstanceRef();
        EXPECT_EQ(4, foo.Get());
    }
    {
        Foo *foo = Singleton<Foo>::Instance();
        EXPECT_EQ(4, foo->Get());
    }
}
