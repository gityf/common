// Copyright (c) 2015 Wang Yaofu.
// All right reserved.
//
// Author: Wang Yaofu voipman@qq.com
// Created: 2015/06/23
// Description: The header file of class ThreadLocal.
//

#pragma once
#include <pthread.h>
#include "uncopyable.h"

template<typename T>
class ThreadLocal : public Uncopyable {
public:
    ThreadLocal() {
        pthread_key_create(&mKey, &ThreadLocal::destructor);
    }

    ~ThreadLocal() {
        pthread_key_delete(mKey);
    }

    T& value() {
        T* perThreadValue = static_cast<T*>(pthread_getspecific(mKey));
        if (!perThreadValue) {
            T* newObj = new T();
            pthread_setspecific(pkey_, newObj);
            perThreadValue = newObj;
        }
        return *perThreadValue;
    }

private:

    static void destructor(void *aObj) {
        T* obj = static_cast<T*>(aObj);
        delete obj;
    }

private:
    pthread_key_t mKey;
};
#endif
