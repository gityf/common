// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/04/2015
// Description: Functions for the manipulation of strings.

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "format.h"
#include "exception.h"

using namespace std;

namespace common {

struct va_ender {
    va_ender(va_list & ap)
        : ap(ap) {
    }

    ~va_ender() {
        va_end(ap);
    }

    va_list & ap;
};

std::string formatImpl(const char * fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    try {
        string result = vformat(fmt, ap);
        va_end(ap);
        return result;
    }
    catch (...) {
        va_end(ap);
        throw;
    }
}

std::string vformat(const char * fmt, va_list ap) {
    char * mem;
    string result;
    int res = vasprintf(&mem, fmt, ap);
    if (res < 0)
        throw Exception("format(): vasprintf error on %s", fmt);

    try {
        result = mem;
        free(mem);
        return result;
    } catch (...) {
        free(mem);
        throw;
    }
}

} // namespace common
