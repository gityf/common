#pragma once
#include <set>
#include "codec.h"

// http protocol message.
struct HttpCodec : public CodecBase{
    HttpCodec() {}
    int tryDecode(Slice data, Slice& msg) override;
    void encode(Slice msg, Buffer& buf) override;
    CodecBase* clone() override { return new HttpCodec(); }
};

// end of local file.
