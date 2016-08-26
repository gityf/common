/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class Socket.
*/

#ifndef _COMMON_NET_SOCKET_H_
#define _COMMON_NET_SOCKET_H_
#include <netinet/in.h>
#include <sys/un.h>
#include <string>
#include "base/localdef.h"
#include "sockutils.h"

using std::string;
struct SAddrInfo {
    string mAddr;
    unsigned short mPort;
    union sockaddr_union mSockUnion;
    int mProtocol;
    int mSockId;
    string mReqCmd;
    string mPeerKey;
    int mFamily;
    string mService;
};

enum EMultiCaseOption {
    kJoinMultiCast = 0,
    kLeaveMultiCast
};

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

class Socket {
public:
    // constructor.
    explicit Socket();

    // Destructor.
    ~Socket();

    // init data when socket start.
    int InitData();

    // name to ipv4 or ipv6
    int Name2IP(string& aAddrName, int aFamily = AF_INET);

    // from string ip port to sockaddr_union
    int InitSockAddrUn(SAddrInfo* aAddrInfo);

    // get port of ipv6 and ipv4.
    unsigned short SuGetPort(union sockaddr_union* su);

    // sets the port number (host byte order)
    void SuSetPort(union sockaddr_union* su, unsigned short port);

    // size of sockaddr_union
    int SuSize(const sockaddr_union& su);

    // init sockaddr_un, socket.
    int CreateSocket(SAddrInfo* aAddrInfo);

    // setsockopt, bind. listen is plus for TCP serever.
    int ListenServer(SAddrInfo* aAddrInfo);

    // tcp socket server��socket bind listen.
    int TcpServer(SAddrInfo* aAddrInfo);

    // tcp IPv6 socket server��socket bind listen.
    int Tcp6Server(SAddrInfo* aAddrInfo);

    // tcp unix socket server��socket bind listen.
    int TcpUnixServer(SAddrInfo* aAddrInfo);

    // connect for TCP client.
    int TcpConnect(const SAddrInfo& aAddrInfo);

    // close the socket
    void CloseSocket(int aSockId);

    // shutdown socket id.
    void ShutdownWrite(int aSockId);

    // make connect to tcp server.
    int MakeTcpConn(SAddrInfo* aAddrInfo, bool aIsBlock = true);

    // make unix local connect to tcp server.
    int MakeUnixConnect(SAddrInfo* aAddrInfo);

    // accept for TCP Server.
    int AcceptConnection(int aSockId, SAddrInfo* aClntAddrInfo);

    // SAddrInfo to ip:port string:int.
    int SAddrInfo2String(SAddrInfo* aClntAddrInfo);

    // host name to ipv4 or ipv6.
    bool HostResolve(const char* aInHost, char *aOutIP, int aOutSize,
                     bool aIsIPOnly = false);

    // host to ip.
    bool Name2IPv4(const char* aInHost, char *aOutIPv4);

    // whether ipstr with ipv4 format
    bool IsIPv4Addr(const string &aIpStr);

    // whether ipstr with ipv6 format
    bool IsIPv6Addr(const string &aIpStr);

    // get peer ip and port by socket id.
    int NetPeerToStr(int aSockId, char *aIp, size_t aIpLen, int *aPort);

    // get local ip and port by socket id.
    int NetLocalSockName(int aSockId, char *aIp, size_t aIpLen, int *aPort);

    // fcntl socket id to NO_BLOCK or BLOCK.
    // NON_BLOCK if aIssetNonBlock is true, otherwise BLOCK.
    int setNonBlock(int aSockId, bool aIssetNonBlock);

    // tcp keep alive option is set on Linux.
    int KeepAlive(int aSockId, int aInterval);

    // whether socket id is set to no-delay.
    int SetTcpNoDelay(int aSockId, int aVal);

    // setTcpQuickAck
    bool setTcpQuickAck(int fd, bool quickAck);

    // setIntOption
    bool setIntOption(int fd, int option, int value);

    // TCP buffer size is set to aBuffSize.
    int SetSendBuffer(int aSockId, int aBuffSize);

    // TCP buffer size is set to aBuffSize.
    int SetRecvBuffer(int aSockId, int aBuffSize);

    // SO_BROADCAST is set for broadcast.
    int SetBroadCast(int aSockId, int aBroadCast);

    // IP_ADD_MEMBERSHIP or IP_DROP_MEMBERSHIP is set for multicast.
    int SetMultiCast(int aSockId, EMultiCaseOption aFlag, const string& aMultiAddress);

    // aFlag = 1:add into group. aFalg=0:drop from group.
    // aIfName is interface name, such as 'etho'.
    int SetMultiCastIpv6(int aSockId, EMultiCaseOption aFlag,
                         const struct sockaddr_in6 *aSin6, const char *aIfName);

    // SO_REUSEADDR or SO_REUSEPORT is set for TCP
    int SetReuseAddr(int aSockId);

    // SO_LINGER is set for TCP
    int SetLinger(int aSockId);

    // IPV6_V6ONLY is set for IPPROTO_IPV6
    int SetV6Only(int aSockId);

    // SO_RCVTIMEO and SO_SNDTIMEO are set for UDP.
    // aFlag=FD_SET_SO_RCVTIMEO|FD_SET_SO_SNDTIMEO
    // aTimeOut=ms
    int SetSockTimeOut(int aSockId, int aFlag, int aTimeOut);

    // get socket error
    int getSoError(int fd);

    // Like read(2) but make sure 'count' is read before to return
    // (unless error or EOF condition is encountered)
    int NetRead(int aSockId, char *aBuf, int aCount);

    // Like write(2) but make sure 'count' is read before to return
    // (unless error is encountered)
    int NetWrite(int aSockId, const char* aBuf, int aCount);

    // no block read
    // no block write

    // select socket, aMask=FD_READABLE|FD_WRITABLE|FD_RWERROR
    int SelectSockId(int aSockId, int aTimeOut, int aMask);

    // poll socket, aMask=FD_READABLE|FD_WRITABLE
    int PollSockId(int aSockId, long long aMillisecs, int aMask);

    // udp sending.
    int UdpSend(int fd, char* buffer, int b_size,
                int timeout, union sockaddr_union* su_cliaddr);

    // udp receiving.
    int UdpRecv(int fd, char *pkg_buffer, int b_size,
                int timeout, union sockaddr_union* su_cliaddr);

    // tcp sending by write. aTimeOut is ms
    int TcpSend(int aSockId, const char* aBuffer,
                int aBufSize, int aTimeOut, bool aHasPolled = false);

    // tcp sending by writev. aTimeOut is ms.
    int TcpSendV(int aSockId, const struct iovec* aIovec,
                 int aIovCnt, int aTimeOut, bool aHasPolled = false);

    // tcp receiving. aTimeOut is ms.
    int TcpRecv(int aSockId, char *aBuffer,
                int aBufSize, int aTimeOut,
                bool aHasPolled = false, int aMaxSize = MAX_SIZE_1M);

    // tcp receiving.
    int TcpRecvOnce(int aSockId, char *aBuffer, int aBufSize);

    // recv buffer max to bufffer size. aTimeOut is ms
    int TcpRecvAll(int aSockId, char *aBuffer, int aBufSize, int aTimeOut);

    // tcp receiving end of \r\n\r\n. aTimeOut is ms
    int TcpRecvHttpHeader(int aSockId, char *aBuffer,
                          int aBufSize, int aTimeOut,
                          bool aHasPolled = false);
    // recv http header and body.
    int RecvHttpPkg(int aSockId, char *aBuffer,
                    int aBufSize, int aTimeOut,
                    bool aHasPolled = false);

    // recv buffer by line,end of '\n'. aTimeOut is ms
    int TcpRecvLine(int aSockId, char *aBuffer, int aBufSize, int aTimeOut);

    // tcp receiving by len-body.
    // aFlag=0:recv buffer by length 'aBufSize'
    // aFlag=10,16:recv buffer by 4 octets len-header.
    // aTimeOut is ms
    int TcpRecvByLen(int aSockId, char *aBuffer, int aBufSize,
                     int aFlag, int aTimeOut, bool aHasPolled = false);

    int CreatePkgHeader(int aLen, char *aOutBuf, const string& aAppStr = "");
    // hex string to int.
    unsigned int LenXHex2Int(const char* _s, int len = 4);

    // hex to int.
    int Hex2Int(const char hex_digit);

    // getting local ip address.
    int GetLocalIP(char* aOutIP);

    // return true if success.
    bool GetTcpInfo(int fd, struct tcp_info*) const;
    bool GetTcpInfoString(int fd, char* buf, int len) const;

    // is ip address localhost.
    bool HostNameIsLocal(const string& aName);
};

#endif  // _COMMON_CSOCKET_H_
