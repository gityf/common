// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/04/2015
// Description: Functions for the manipulation of strings.

#ifndef _COMMON_FORMAT_H_
#define _COMMON_FORMAT_H_

#include <stdarg.h>
#include <string>

namespace common {
#define ALWAYS_INLINE __attribute__((__always_inline__)) inline 
// This machinery allows us to use a std::string with %s via c++11
template<typename T>
ALWAYS_INLINE T forwardForPrintf(T t) {
    return t;
}

ALWAYS_INLINE const char * forwardForPrintf(const std::string & s) {
    return s.c_str();
}

std::string formatImpl(const char * fmt, ...);

template<typename... Args>
ALWAYS_INLINE std::string format(const char * fmt, Args... args) {
    return formatImpl(fmt, forwardForPrintf(args)...);
}

inline std::string format(const char * fmt)
{
    return fmt;
}

std::string vformat(const char * fmt, va_list ap);

} // namespace common

#endif // _COMMON_FORMAT_H_
