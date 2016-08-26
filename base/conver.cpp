// 2011-10-11
// xcore_conver.cpp
//
// ×Ö·û¼¯×ª»»

#include "conver.h"
#include <iconv.h>
#include <errno.h>
namespace common {
bool isGbk(const unsigned char c1, const unsigned char c2) {
    return ((c1 >= 0x81 && c1 <= 0xFE) && \
           ((c2 >= 0x40 && c2 <= 0x7E) || \
            (c2 >= 0xA1 && c2 <= 0xFE))) ? true : false;
}
bool isUtf8(const char *data, unsigned int length, bool nullCheck) {
    unsigned int i = 0;
    while (i < length) {
        unsigned char b = data[i++];
        // Check the first octet of the current UTF8 character.
        // Null check
        if (b == 0x00) {
            if (nullCheck) {
                return false;
            }
        } else if ((b >> 7) == 0x00) {
            //7-bit ASCII character.
            continue;
        } else if ((b >> 1) == 0x7F) {
            // If the first 7 bits are all '1's, this is not an UTF8 character.
            return false;
        }
        //Count the number of '1' of the first octet
        int count = 0;
        for (; count < 5; count++) {
            if ((b & 0x40) == 0)  {
                break;
            }
            b <<= 1;
        }
        // The count value must be greater than 0.
        if (count == 0) {
            //Out of UTF8 character range.
            return false;
        }
        // Check remaining octet(s)
        for (int j = 0; j < count; j++) {
            if (i >= length) {
                return false;
            }
            if ((data[i++] >> 6) != 0x02) {
                return false;
            }
        }
    }
    return true;
}
bool gbk_to_utf8(std::string& result, const std::string& str)
{
    result.clear();

    iconv_t icd = iconv_open("UTF-8", "GBK");
    if (icd == (iconv_t)-1) return false;

    char buf[1024];
    size_t nSrcLen = str.size();
    char* in_ = (char *)str.c_str();
    while (nSrcLen > 0)
    {
        size_t nDstLen = 1024;
        char* out_ = buf;
        size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
        if (ret == (size_t)-1 && errno != E2BIG)
        {
            iconv_close(icd);
            return false;
        }
        result.append(buf, 1024 - nDstLen);
    }
    iconv_close(icd);
    return true;
}

bool gbk_to_unicode(std::wstring& result, const std::string& str)
{
    result.clear();

    iconv_t icd = iconv_open("UNICODE", "GBK");
    if (icd == (iconv_t)-1) return false;

    wchar_t buf[1024];
    size_t nSrcLen = str.size();
    char* in_ = (char *)str.c_str();
    while (nSrcLen > 0)
    {
        size_t nDstLen = 2048;
        char* out_ = (char*)buf;
        size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
        if (ret == (size_t)-1 && errno != E2BIG)
        {
            iconv_close(icd);
            return false;
        }
        result.append(buf, (2048 - nDstLen) / 2);
    }
    iconv_close(icd);
    return true;
}

bool utf8_to_gbk(std::string& result, const std::string& str)
{
    result.clear();

    iconv_t icd = iconv_open("GBK", "UTF-8");
    if (icd == (iconv_t)-1) return false;

    char buf[1024];
    size_t nSrcLen = str.size();
    char* in_ = (char *)str.c_str();
    while (nSrcLen > 0)
    {
        size_t nDstLen = 1024;
        char* out_ = buf;
        size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
        if (ret == (size_t)-1 && errno != E2BIG)
        {
            iconv_close(icd);
            return false;
        }
        result.append(buf, 1024 - nDstLen);
    }
    iconv_close(icd);
    return true;
}

bool utf8_to_unicode(std::wstring& result, const std::string& str)
{
    result.clear();

    iconv_t icd = iconv_open("UNICODE", "UTF-8");
    if (icd == (iconv_t)-1) return false;

    wchar_t buf[1024];
    size_t nSrcLen = str.size();
    char* in_ = (char *)str.c_str();
    while (nSrcLen > 0)
    {
        size_t nDstLen = 2048;
        char* out_ = (char*)buf;
        size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
        if (ret == (size_t)-1 && errno != E2BIG)
        {
            iconv_close(icd);
            return false;
        }
        result.append(buf, (2048 - nDstLen) / 2);
    }
    iconv_close(icd);
    return true;
}

bool unicode_to_gbk(std::string& result, const std::wstring& str)
{
    result.clear();

    iconv_t icd = iconv_open("GBK", "UNICODE");
    if (icd == (iconv_t)-1) return false;

    char buf[1024];
    size_t nSrcLen = str.size() * 2;
    char* in_ = (char *)str.c_str();
    while (nSrcLen > 0)
    {
        size_t nDstLen = 1024;
        char* out_ = (char*)buf;
        size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
        if (ret == (size_t)-1 && errno != E2BIG)
        {
            iconv_close(icd);
            return false;
        }
        result.append(buf, 1024 - nDstLen);
    }
    iconv_close(icd);
    return true;
}

bool unicode_to_utf8(std::string& result, const std::wstring& str)
{
    result.clear();

    iconv_t icd = iconv_open("UTF-8", "UNICODE");
    if (icd == (iconv_t)-1) return false;

    char buf[1024];
    size_t nSrcLen = str.size() * 2;
    char* in_ = (char *)str.c_str();
    while (nSrcLen > 0)
    {
        size_t nDstLen = 1024;
        char* out_ = (char*)buf;
        size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
        if (ret == (size_t)-1 && errno != E2BIG)
        {
            iconv_close(icd);
            return false;
        }
        result.append(buf, 1024 - nDstLen);
    }
    iconv_close(icd);
    return true;
}
} // namespace common
