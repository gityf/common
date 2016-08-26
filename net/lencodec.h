#pragma once
#include "codec.h"

//给出长度的消息
struct LengthCodec : public CodecBase {
    int tryDecode(Slice data, Slice& msg) override;
    void encode(Slice msg, Buffer& buf) override;
    CodecBase* clone() override { return new LengthCodec(); }
};
// end of local file.
