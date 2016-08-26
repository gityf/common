#include <string>
#include "tcpserver.h"
#include "httpcodec.h"
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        return -1;
    }
    TcpServer tcp;
    tcp.initData();
    tcp.initCallBack();
    tcp.getBufferEvent()->setCodec(new HttpCodec);
    tcp.setTcpCallback([](int aSockid, Slice& aRequ, Buffer& aResp) {
        aResp.append("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ");
        aResp.append(std::to_string(aRequ.size())).append("\r\n\r\n");
        aResp.append(aRequ.data(), aRequ.size());
        return RET_OK;
    });
    int listenfd = tcp.bind(argv[1], atoi(argv[2]));
    tcp.setAcceptInfo(listenfd);
    tcp.start();
    return 0;
}