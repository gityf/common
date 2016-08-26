#include "exception.h"
#include "format.h"
#include <string.h>

using namespace std;

namespace common {

Exception::Exception(const std::string & msg)
    : mMessage(msg)
{
    mMessage.c_str();  // make sure we have a null terminator
}

Exception::Exception(const char * msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    try {
        mMessage = vformat(msg, ap);
        mMessage.c_str();
        va_end(ap);
    }
    catch (...) {
        va_end(ap);
        throw;
    }
}

Exception::Exception(const char * msg, va_list ap)
{
    mMessage = vformat(msg, ap);
    mMessage.c_str();
}

Exception::
Exception(int errnum, const std::string & msg, const char * function)
{
    string error = strerror(errnum);

    if (function) {
        mMessage = function;
        mMessage += ": ";
    }

    mMessage += msg;
    mMessage += ": ";

    mMessage += error;

    mMessage.c_str();
}

Exception::~Exception() throw()
{
}

const char * Exception::what() const throw()
{
    return mMessage.c_str();
}

} // namespace common
