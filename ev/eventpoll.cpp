/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CEventPoll.
*/
#include "eventpoll.h"
#include "eventloop.h"
/*
#ifdef __linux__
#include "polls/cepoll.cpp"
#else
#include "polls/cselect.cpp"
#endif
//*/
/* Test for polling API */
#ifdef __linux__
#define HAVE_EPOLL 1
#endif

#if (defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)) \
  || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#define HAVE_KQUEUE 1
#endif

#ifdef USE_POLL
#include "polls/cpoll.cpp"
#else
    #ifdef HAVE_EPOLL
    #include "polls/cepoll.cpp"
    #else
        #ifdef HAVE_KQUEUE
        #include "polls/ckqueue.cpp"
        #else
        #include "polls/cselect.cpp"
        #endif
    #endif
#endif
