#ifndef _COMMON_EXCEPTION_H_
#define _COMMON_EXCEPTION_H_

#include <stdarg.h>
#include <string>
#include <exception>

namespace common {

class Exception : public std::exception {
public:
    Exception(const std::string & msg);
    Exception(const char * msg, ...);
    Exception(const char * msg, va_list ap);
    Exception(int errnum, const std::string & msg, const char * function = 0);
    virtual ~Exception() throw();

    virtual const char * what() const throw();

private:
    std::string mMessage;
};

} // namespace common

#endif // _COMMON_EXCEPTION_H_
