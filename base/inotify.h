/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Header file of class Inotify.
*/

#pragma once
#include <sys/inotify.h>
#include <string>
#include <map>
#include <functional>
using std::string;

// callback function: param 1-EventType, param 2: watch file.
using WatchFuncPtr = std::function<void(unsigned int, const string&)>;
// idle callback function.
using IdleFuncPtr = std::function<void(void)>;
class Inotify {
public:
    Inotify ();
    ~Inotify ();
    // add watch file.
    void addWatchFile(const string& file, WatchFuncPtr&& funcPtr) {
        fileWatchs_[file] = std::move(funcPtr);
    }
    //
    void addWatchFile(const string& file, WatchFuncPtr& funcPtr) {
        addWatchFile(file, WatchFuncPtr(funcPtr));
    }
    // add idle function ptr.
    void setIdleFunc(IdleFuncPtr&& funcPtr) {
        idleFuncPtr_ = std::move(funcPtr);
    }
    //
    void setIdleFunc(IdleFuncPtr& funcPtr) {
        setIdleFunc(IdleFuncPtr(funcPtr));
    }
    // run loop.
    void loop(long waitMs = 2000);
    // stop loop
    void stop();
    // set mask and name pair;
    void set(uint32_t mask, const string& name) {
        mask2names_[mask] = name;
    }
    // set name and mask pair;
    void set(const string& name, uint32_t mask) {
        name2masks_[name] = mask;
    }
    // get name by mask.
    string get(uint32_t mask) {
        if (mask2names_.find(mask) != mask2names_.end()) {
            return mask2names_[mask];
        }
        return string("");
    }
    // get mask by name.
    uint32_t get(const string& name) {
        if (name2masks_.find(name) != name2masks_.end()) {
            return name2masks_[name];
        }
        return 0;       
    }
private:
    struct inotify_event *inotify_next_events(long int timeout, int num_events);
private:
    // file AND watch function pair.
    std::map<string, WatchFuncPtr> fileWatchs_;
    std::map<uint32_t, string> mask2names_;
    std::map<string, uint32_t> name2masks_;
    bool isStoped_;
    // idle function ptr.
    IdleFuncPtr idleFuncPtr_;
    // inotify fd.
    int inotifyfd_;
};
