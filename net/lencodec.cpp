#include <string>
#include "lencodec.h"
#include "base/stringutils.h"

int LengthCodec::tryDecode(Slice data, Slice& msg) {
    if (data.size() < 8) {
        return 0;
    }
    int len = common::StrUtils::HexStr2Int(data.data() + 4);
    if (len < 0 || memcmp(data.data(), "'YF'", 4) != 0) {
        return -1;
    }
    if (data.size() >= len + 8) {
        msg = Slice(data.data() + 8, len);
        return len + 8;
    }
    return 0;
}

void LengthCodec::encode(Slice msg, Buffer& buf) {
    buf.append("'YF'").append(std::to_string(msg.size())).append(msg);
}
// end of local file.