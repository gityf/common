#include <iostream>
#include "base/stack_trace.h"
#include "ut/test_harness.h"


TEST(StackTrace, BasicTest) {
    common::StackTrace st;
    std::string ststr;
    st.ToString(&ststr);
    std::cout << ststr << std::endl;
}
