#pragma once
#include "slice.h"
#include "buffer.h"

struct CodecBase {
    // > 0 decode all message,
    // the decode msg is pushed into msg, size of parased is return.
    // == 0 decode part message.
    // < 0 decode error
    virtual int tryDecode(Slice data, Slice& msg) = 0;
    virtual void encode(Slice msg, Buffer& buf) = 0;
    virtual CodecBase* clone() = 0;
};
// end of local file.
