#pragma once
#include "codec.h"

//以\r\n结尾的消息
struct LineCodec : public CodecBase{
    int tryDecode(Slice data, Slice& msg) override;
    void encode(Slice msg, Buffer& buf) override;
    CodecBase* clone() override { return new LineCodec(); }
};

// end of local file.
