#pragma once
#include <string.h>
#include <string>
#include <algorithm>
#include "slice.h"

struct Buffer {
    static const size_t kInitialSize = 4096;
    Buffer() : buffer_(NULL), b_(0), e_(0), cap_(0), exp_(4096) {}
    ~Buffer() {}
    void clear() {
        b_ = e_ = 0;
    }
    size_t size() const {
        return e_ - b_;
    }
    bool empty() const  {
        return e_ == b_;
    }
    char* data() const  {
        return buffer_ + b_;
    }
    char* begin() const  {
        return buffer_ + b_;
    }
    char* end() const  {
        return buffer_ + e_;
    }
    void setEnd() {
        buffer_[e_] = 0x00;
    }
    void makeRoom() {
        if (e_ >= cap_) expand(0);
    }
    size_t space() const  {
        return cap_ - e_;
    }
    void addSize(size_t len) {
        e_ += len;
    }
    Buffer& append(const char* p, size_t len) {
        if (len > space()) {
            expand(len);
        }
        memcpy(end(), p, len);
        addSize(len);
        return *this;
    }
    Buffer& append(Slice slice) {
        return append(slice.data(), slice.size());
    }
    Buffer& append(const char* p) {
        return append(p, strlen(p));
    }
    Buffer& append(const std::string& str) {
        append(str.data(), str.length());
    }

    template<class T>
    Buffer& appendValue(const T& v) {
        append((const char*)&v, sizeof v);
        return *this;
    }
    Buffer& consume(size_t len) {
        if (e_ > b_) {
            b_ += len;
            if (size() == 0){
                clear();
            }
        }
        return *this;
    }
    void setSuggestSize(size_t sz) {
        exp_ = sz;
    }
    Buffer(const Buffer& b) {
        copyFrom(b);
    }

    Buffer& operator=(const Buffer& b) {
        copyFrom(b);
        return *this;
    }

    void swap(Buffer& rhs)
    {
        std::swap(buffer_, rhs.buffer_);
        std::swap(b_, rhs.b_);
        std::swap(e_, rhs.e_);
        std::swap(cap_, rhs.cap_);
        std::swap(exp_, rhs.exp_);
    }
    operator Slice () {
        return Slice(data(), size());
    }
private:
    char* buffer_;
    size_t b_, e_, cap_, exp_;
    void expand(size_t len) {
        size_t ncap = std::max(exp_, std::max(2 * cap_, size() + len));
        if (buffer_ == NULL) {
            buffer_ = reinterpret_cast<char*>(malloc(ncap));
        }
        else {
            buffer_ = reinterpret_cast<char *>(realloc(buffer_, ncap));
        }
        cap_ = ncap;
    }
    void copyFrom(const Buffer& b) {
        if (space() < b.size()) {
            expand(b.size());
        }
        memcpy(begin(), b.begin(), b.size());
    }
};
// end of local file.