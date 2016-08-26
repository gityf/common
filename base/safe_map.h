/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CSafeMap
*/

#ifndef _COMMON_SAFE_MAP_H_
#define _COMMON_SAFE_MAP_H_

#include <map>
#include "thread.h"
template<typename T1, typename T2>
class SafeMap {
public:
    SafeMap() { }
    ~SafeMap() { }

    void insert(T1 key, T2 value) {
        ScopeLock lock(mMutex);
        map_obj.insert(std::map<T1, T2>::value_type(key, value));
    }
    
    void erase(T1 key) {
        ScopeLock lock(mMutex);
        if (map_obj.find(key) != map_obj.end()) {
            map_obj.erase(key);            
        }
    }

    T2 find(T1 key, T2 default_value) {
        ScopeLock lock(mMutex);
        auto iter = map_obj.find(key);
        return (iter != map_obj.end()) ? iter->second : default_value;
    }

private:
    Mutex mMutex;
    std::map<T1, T2> map_obj;
};
#endif // _COMMON_SAFE_MAP_H_
