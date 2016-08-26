#include "linecodec.h"

int LineCodec::tryDecode(Slice data, Slice& msg) {
    for (size_t i = 0; i < data.size(); i++) {
        if (data[i] == '\n') {
            if (i > 0 && data[i - 1] == '\r') {
                msg = Slice(data.data(), i - 1);
                return i + 1;
            }
            else {
                msg = Slice(data.data(), i);
                return i + 1;
            }
        }
    }
    return 0;
}

void LineCodec::encode(Slice msg, Buffer& buf) {
    buf.append("\r\n");
}
// end of local file.