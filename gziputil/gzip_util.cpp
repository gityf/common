#include "gzip_util.h"
#include "GZipHelper.h"
#include <sstream>
#include <iostream>
#include <sstream>

using namespace std;
namespace common {
#define ZIP_BUFFER_SIZE 262144

int GZipUtil::GZip(const std::string& str, std::string& out) {
    char* pStr = (char*)str.c_str();
    // do compressing here
    CA2GZIPT<ZIP_BUFFER_SIZE, Z_BEST_COMPRESSION, Z_DEFAULT_STRATEGY> gzip(pStr, str.length());
    out = string((char*) gzip.pgzip, gzip.Length);
    return 0;
}

int GZipUtil::GUnZip(const std::string& str, std::string& out) {
    int cclen = str.length();
    // do uncompressing here
    CGZIP2AT<ZIP_BUFFER_SIZE> plain((unsigned char *) str.c_str(), cclen);
    out = string((char*) plain.psz, plain.Length);
    return 0;
}

}
