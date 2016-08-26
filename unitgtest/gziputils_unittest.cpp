#include "gziputil/gzip_util.h"
#include "ut/test_harness.h"
#include "base/stringutils.h"
#include <iostream>

using namespace std;

TEST(TestGzipUtil, BasicTest) {
    string sstr;
    string sout;
    sstr = "This is test string for gzip.";
    common::GZipUtil::GZip(sstr, sout);
    string hexZipOut = common::StrUtils::Hex(sout);
    cout << hexZipOut << endl;
    string unzipStr;
    common::GZipUtil::GUnZip(sout, unzipStr);
    EXPECT_EQ(sstr, unzipStr);
}

