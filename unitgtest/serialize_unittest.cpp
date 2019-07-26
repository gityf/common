/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of serialize ut.
*/
#include <iostream>
#include <sstream>
#include "base/serialize.h"
#include "base/stringutils.h"
#include "ut/test_harness.h"
using namespace std;
using namespace common;

class TestSerializable : public Serializable {
public:
     /**
     * Serialize to a stream.
     */
    void Serialize(std::ostream& os) {
		common::Serialize(iInt, os);
		common::Serialize(iFloat, os);
		common::Serialize(iDouble, os);
		common::Serialize(iStr, os);
		common::Serialize(iLong, os);
    }
    /**
     * De-serialize from a stream.
     */
    void Deserialize(std::istream& is) {
        common::Deserialize(iInt, is);
        common::Deserialize(iFloat, is);
        common::Deserialize(iDouble, is);
        common::Deserialize(iStr, is);
        common::Deserialize(iLong, is);
    }

    void DumpMember() {
        cout << "int: " << iInt << endl
            << "float: " << iFloat << endl
            << "double: " << iDouble << endl
            << "string: " << iStr << endl
            << "long: " << iLong << endl;
    }
    int iInt;
    float iFloat;
    double iDouble;
    string iStr;
    long iLong;

};

TEST(SerializeTest, SerializeA)
{
    TestSerializable tt;
    TestSerializable dd;
    tt.iInt = 123;
    tt.iFloat = 5.456;
    tt.iDouble = 6.876;
    tt.iLong = 987654321L;
    tt.iStr = "This is serialize string.";
    ostringstream ostr;
    try {
        tt.Serialize(ostr);
        cout << "Serialize:>>" << StrUtils::Hex(ostr.str()) << endl;
        tt.DumpMember();
        string rstr = ostr.str();
       
        istringstream istr(rstr);
        dd.Deserialize(istr);
        cout << "Deserialize:>>" << StrUtils::Hex(istr.str()) << endl;
        dd.DumpMember();
    } catch (string &e) {
        cout << "ERROR:" << e << endl;
    }
    EXPECT_EQ(tt.iInt, dd.iInt);
    EXPECT_EQ(tt.iFloat, dd.iFloat);
    EXPECT_EQ(tt.iDouble, dd.iDouble);
    EXPECT_EQ(tt.iLong, dd.iLong);
    EXPECT_EQ(tt.iStr, dd.iStr);
    getchar();
}


