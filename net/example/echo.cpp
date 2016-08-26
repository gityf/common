#include "tcpserver.h"
#include "linecodec.h"
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        return -1;
    }
    TcpServer tcp;
    tcp.initData();
    tcp.initCallBack();
    tcp.getBufferEvent()->setCodec(new LineCodec);
    tcp.setTcpCallback([](int aSockid, Slice& aRequ, Buffer& aResp) {
        aResp.append(aRequ).append("\r\n");
        return RET_OK;
    });
    int listenfd = tcp.bind(argv[1], atoi(argv[2]));
    tcp.setAcceptInfo(listenfd);
    tcp.start();
    return 0;
}