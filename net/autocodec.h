#pragma once
#include <functional>

struct AutoContext {
    void* ctx;
    std::function<void()> ctxDel;
    AutoContext():ctx(0) {}
    template<class T>
    T& context() {
        if (ctx == NULL) {
            ctx = new T();
            ctxDel = [this] { delete (T*)ctx; };
        }
        return *(T*)ctx;
    }
    ~AutoContext() { if (ctx) ctxDel(); }
};
// end of local file.