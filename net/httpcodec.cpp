#include <string>
#include "httpcodec.h"
#include "httputils.h"

int HttpCodec::tryDecode(Slice data, Slice& msg) {
    // POST GET HTTP/1.1
    const char* lineEndPtr = strstr(data.data(), common::HTTP_CRLF.c_str());
    if (lineEndPtr == NULL) {
        return 0;
    }
    const char* endPtr = common::HttpUtils::GetBodyStr(lineEndPtr);
    if (endPtr == NULL) {
        return 0;
    }
    // GET HEAD
    if (data[0] == 'G') {
        msg = Slice(data.data(), endPtr);
        msg.setHttpPtr(lineEndPtr, endPtr);
        return endPtr - data.data();
    }
    // POST PUT HTTP/1.1  DELETE
    else if (data[0] == 'P' || data[0] == 'H') {
        int bodyLen = common::HttpUtils::GetHeaderValue(data.data(),
            "Content-Length");
        if (bodyLen >= 0) {
            endPtr += bodyLen;
            if (data.end() >= endPtr) {
                msg = Slice(data.begin(), endPtr);
                msg.setHttpPtr(lineEndPtr, endPtr - bodyLen);                
                return endPtr - data.begin();
            }
        }
    }
    else {
        return -1;
    }
    return 0;
}

void HttpCodec::encode(Slice msg, Buffer& buf) {
    buf.append(msg);
}
// end of local file.
