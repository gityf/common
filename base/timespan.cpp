/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Source file of class Timespan.
*/
#include "timespan.h"
#include <sys/time.h>
#include <time.h>
#include <iostream>

namespace common {
    Timespan Timespan::gettimeofday() {
        timeval tv;
        ::gettimeofday(&tv, 0);
        return Timespan(tv.tv_sec, tv.tv_usec);
    }

    std::ostream& operator<< (std::ostream& out, const Timespan& ht)
    {
        out << static_cast<double>(ht.toUSecs()) / 1e6;
        return out;
    }
}
