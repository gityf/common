/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class Inotify.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "inotify.h"

namespace {
    struct timeval* timevalCnv(long ms, struct timeval& tval) {
        struct timeval* tvalp;
        // set select timeout
        tvalp = NULL;
        if (ms > 0) {
            tval.tv_sec = ms / 1000;
            tval.tv_usec = (ms % 1000) * 1000;
            if (tval.tv_usec >= 1000000) {
                tval.tv_sec++;
                tval.tv_usec -= 1000000;
            }
            tvalp = &tval;
        }
        return tvalp;
    }
}
#define INOTIFY_NAME(a, b)  do { a->set(b, #b); a->set(#b, b); } while (0);
#define MAX_EVENTS 512

Inotify::Inotify() {
    isStoped_ = false;
    INOTIFY_NAME(this, IN_ACCESS);
    INOTIFY_NAME(this, IN_MODIFY);
    INOTIFY_NAME(this, IN_ATTRIB);
    INOTIFY_NAME(this, IN_CLOSE_WRITE);
    INOTIFY_NAME(this, IN_CLOSE_NOWRITE);
    INOTIFY_NAME(this, IN_OPEN);
    INOTIFY_NAME(this, IN_MOVED_FROM);
    INOTIFY_NAME(this, IN_MOVED_TO);
    INOTIFY_NAME(this, IN_CREATE);
    INOTIFY_NAME(this, IN_DELETE);
    INOTIFY_NAME(this, IN_DELETE_SELF);
    INOTIFY_NAME(this, IN_UNMOUNT);
    INOTIFY_NAME(this, IN_Q_OVERFLOW);
    INOTIFY_NAME(this, IN_IGNORED);
    INOTIFY_NAME(this, IN_CLOSE);
    INOTIFY_NAME(this, IN_MOVE);
    INOTIFY_NAME(this, IN_ISDIR);
    INOTIFY_NAME(this, IN_ONESHOT);
    INOTIFY_NAME(this, IN_ALL_EVENTS);
    INOTIFY_NAME(this, IN_DONT_FOLLOW);
    INOTIFY_NAME(this, IN_ONLYDIR);
    INOTIFY_NAME(this, IN_MOVE_SELF);
}

Inotify ::~Inotify() {
}

struct inotify_event * Inotify::inotify_next_events(long int timeout,
    int num_events) {
    if (num_events < 1) return NULL;

    static struct inotify_event event[MAX_EVENTS];
    static struct inotify_event * ret;
    static int first_byte = 0;
    static ssize_t bytes;


    // first_byte is index into event buffer
    if (first_byte != 0
        && first_byte <= (int)(bytes - sizeof(struct inotify_event))) {

        ret = (struct inotify_event *)((char *)&event[0] + first_byte);
        first_byte += sizeof(struct inotify_event) + ret->len;

        // if the pointer to the next event exactly hits end of bytes read,
        // that's good.  next time we're called, we'll read.
        if (first_byte == bytes) {
            first_byte = 0;
        }
        else if (first_byte > bytes) {
            // how much of the event do we have?
            bytes = (char *)&event[0] + bytes - (char *)ret;
            memcpy(&event[0], ret, bytes);
            return inotify_next_events(timeout, num_events);
        }
        return ret;
    }
    else if (first_byte == 0) {
        bytes = 0;
    }


    static ssize_t this_bytes;
    static unsigned int bytes_to_read;
    static fd_set read_fds;

    static struct timeval read_timeout;
    static struct timeval * read_timeout_ptr;
    read_timeout_ptr = timevalCnv(timeout, read_timeout);

    FD_ZERO(&read_fds);
    FD_SET(inotifyfd_, &read_fds);
    int rc = ::select(inotifyfd_ + 1, &read_fds,
        NULL, NULL, read_timeout_ptr);
    if (rc < 0) {
        return NULL;
    }
    else if (rc == 0) {
        // timeout
        return NULL;
    }

    int maxTimes = 10;
    // wait until we have enough bytes to read
    do {
        rc = ioctl(inotifyfd_, FIONREAD, &bytes_to_read);
        if (--maxTimes <= 0) {
            break;
        }
    } while (!rc &&
        bytes_to_read < sizeof(struct inotify_event)*num_events);

    if (rc == -1) {
        return NULL;
    }

    this_bytes = read(inotifyfd_, &event[0] + bytes,
        sizeof(struct inotify_event)*MAX_EVENTS - bytes);
    if (this_bytes < 0) {
        return NULL;
    }
    if (this_bytes == 0) {
        fprintf(stderr, "Inotify reported end-of-file.  Possibly too many "
            "events occurred at once.\n");
        return NULL;
    }
    bytes += this_bytes;

    ret = &event[0];
    first_byte = sizeof(struct inotify_event) + ret->len;
    if (first_byte == bytes) {
        first_byte = 0;
    }

    return ret;
}

void Inotify::loop(long waitMs) {
    inotifyfd_ = ::inotify_init();
    std::map<int, string> fd2files;
    for (auto it = fileWatchs_.begin();
        it != fileWatchs_.end(); ++it) {
        int wd = ::inotify_add_watch(inotifyfd_, it->first.c_str(), IN_ALL_EVENTS);
        if (wd != -1) {
            fd2files[wd] = it->first;
        }
    }
    while (!isStoped_) {
        struct inotify_event *ev = inotify_next_events(waitMs, fileWatchs_.size());
        if (ev != NULL) {
            string tfile = fd2files[ev->wd];
            if (fileWatchs_.find(tfile) !=
                fileWatchs_.end()) {
                fileWatchs_[tfile](ev->mask, tfile);
            }
        }
        if (idleFuncPtr_) {
            // call idle callback function here.
            idleFuncPtr_();
        }
        sleep(1);
    }
    for (auto it = fd2files.begin();
        it != fd2files.end(); ++it) {
        ::inotify_rm_watch(inotifyfd_, it->first);
    }
}

void Inotify::stop() {
    isStoped_ = true;
}
