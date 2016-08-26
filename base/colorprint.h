/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: This is the header file of colored console UI helper.
*/
#ifndef _COMMON_COLORPRINT_H_
#define _COMMON_COLORPRINT_H_

#include <iostream>

namespace common
{
    inline std::ostream& ColorBlack(std::ostream &s) {
        s << "\033[0;30m";
        return s;
    }

    inline std::ostream& ColorRed(std::ostream &s) {
        s << "\033[0;31m";
        return s;
    }

    inline std::ostream& ColorGreen(std::ostream &s) {
        s << "\033[0;32m";
        return s;
    }

    inline std::ostream& ColorYellow(std::ostream &s) {
        s << "\033[0;33m";
        return s;
    }

    inline std::ostream& ColorBlue(std::ostream &s) {
        s << "\033[0;34m";
        return s;
    }

    inline std::ostream& ColorPurple(std::ostream &s) {
        s << "\033[0;35m";
        return s;
    }

    inline std::ostream& ColorCyan(std::ostream &s) {
        s << "\033[0;36m";
        return s;
    }

    inline std::ostream& ColorWhite(std::ostream &s) {
        s << "\033[0;37m";
        return s;
    }

    inline std::ostream& ColorRestore(std::ostream &s) {
        s << "\033[0m";
        return s;
    }
} 

#endif  // _COMMON_COLORPRINT_H_
