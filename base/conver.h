// ×Ö·û¼¯×ª»»

#ifndef _COMMON_CONVER_H_
#define _COMMON_CONVER_H_

#include <string>
namespace common {
    bool isGbk(const unsigned char c1, const unsigned char c2);
    bool isUtf8(const char *data, unsigned int length, bool nullCheck = false);
    bool gbk_to_utf8(std::string& result, const std::string& str);
    bool gbk_to_unicode(std::wstring& result, const std::string& str);
    bool utf8_to_gbk(std::string& result, const std::string& str);
    bool utf8_to_unicode(std::wstring& result, const std::string& str);
    bool unicode_to_gbk(std::string& result, const std::wstring& str);
    bool unicode_to_utf8(std::string& result, const std::wstring& str);
} // namespace common
#endif // _COMMON_CONVER_H_
