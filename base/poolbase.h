/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class PollBase.
*/

#ifndef _COMMON_POOL_H_
#define _COMMON_POOL_H_
#include <vector>
#include <memory>
#include "thread.h"
// Add the T Pool for reusing T connection.
template <typename Type>
class PollBase {
public:
    PollBase() : mPool(), mPoolLock(), mMaxSize(5) {
    }

    explicit PollBase(size_t aMaxSize) {
        mPool.clear();
        mMaxSize = aMaxSize;
    }

    size_t GetMaxSize() const {
        return mMaxSize;
    }

    Type* Get() {
        Type* handle = NULL;
        {
            ScopedSpinLock lock(mPoolLock);
            if(!mPool.empty()) {
                handle = mPool.back();
                mPool.pop_back();
            }
        }
        // may be null.
        return handle;
    }

    void Put(Type* handle) {
        ScopedSpinLock lock(mPoolLock);
        mPool.push_back(handle);
    }
    // how many handle exist in pool.
    size_t Reserve() const {
        return mPool.size();
    }

    ~PollBase() {
        ScopedSpinLock lock(mPoolLock);
        while(!mPool.empty()) {
            Type* handle = mPool.back();
            mPool.pop_back();
            if (handle != NULL) {
                delete handle;
            }
        }
    }
private:
    std::vector<Type*> mPool;
    SpinLock mPoolLock;
    size_t mMaxSize;
};
typedef std::shared_ptr<PollBase> PollBasePtr;
#endif // _COMMON_POOL_H_