/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class Socket.
*/
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#ifdef SunOS
#include <sys/sockio.h>
#endif
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "socket.h"
#include "base/log.h"

#define PKG_HEADFLAG_LEN  4
#define PKG_CMDLEN_LEN    4
#define PKG_CHKSUM_LEN    8
#define PKG_HEADER_LEN    8
#define PKG_TCP_HEAD_FLAG "'YF'"
// ---------------------------------------------------------------------------
// constructor.
// ---------------------------------------------------------------------------
//
Socket::Socket() {
}

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
Socket::~Socket() {
}

// ---------------------------------------------------------------------------
// init operation where local class is created.
// ---------------------------------------------------------------------------
//
int Socket::InitData() {
    DEBUG(LL_ALL, ":Begin.");
    DEBUG(LL_ALL, "End");
    return 0;
}
// ---------------------------------------------------------------------------
// dns to ip(v4/v6) address.
// ---------------------------------------------------------------------------
//
int Socket::Name2IP(string& aAddrName, int aFamily /* = AF_INET */) {
    if (aAddrName.empty()) {
        return RET_OK;
    }

    switch (aFamily) {
        case AF_INET6:
            {
                if (!IsIPv6Addr(aAddrName)) {
                    char ipstr[64] = {0};
                    if (HostResolve(aAddrName.c_str(), ipstr, 64)) {
                        aAddrName = ipstr;
                    } else {
                        LOG(LL_ERROR, "unknown Dns %s.",
                            aAddrName.c_str());
                        return RET_ERROR;
                    }
                }
            }
            break;
        case AF_INET:
            {
                if (!IsIPv4Addr(aAddrName)) {
                    char ipstr[64] = {0};
                    if (Name2IPv4(aAddrName.c_str(), ipstr)) {
                        aAddrName = ipstr;
                    } else {
                        LOG(LL_ERROR, "unknown Dns %s.",
                            aAddrName.c_str());
                        return RET_ERROR;
                    }
                }
            }
            break;
        default:
            break;
    }
    return RET_OK;
}

// from string ip port to sockaddr_union
int Socket::InitSockAddrUn(SAddrInfo* aAddrInfo) {
    if (aAddrInfo == NULL) return RET_ERROR;
    DEBUG(LL_ALL, "Begin.");
    if (RET_ERROR == Name2IP(aAddrInfo->mAddr)) {
        return RET_ERROR;
    }

    memset(&aAddrInfo->mSockUnion, 0, sizeof(union sockaddr_union));
    switch (aAddrInfo->mFamily) {
        case AF_INET6:
            aAddrInfo->mSockUnion.s.sa_family = PF_INET6;
            if (aAddrInfo->mAddr.empty()) {
                aAddrInfo->mSockUnion.sin6.sin6_addr = in6addr_any;
            } else {
                inet_pton(AF_INET6, aAddrInfo->mAddr.c_str(),
                    &aAddrInfo->mSockUnion.sin6.sin6_addr);
            }
            aAddrInfo->mSockUnion.sin6.sin6_port = htons(aAddrInfo->mPort);
            break;
        case AF_INET:
            aAddrInfo->mSockUnion.s.sa_family = PF_INET;
            if (aAddrInfo->mAddr.empty()) {
                aAddrInfo->mSockUnion.sin.sin_addr.s_addr = INADDR_ANY;
            } else {
                aAddrInfo->mSockUnion.sin.sin_addr.s_addr =
                    inet_addr(aAddrInfo->mAddr.c_str());
            }
            aAddrInfo->mSockUnion.sin.sin_port = htons(aAddrInfo->mPort);
            break;
        case AF_UNIX:
            aAddrInfo->mSockUnion.sun.sun_family = AF_LOCAL;
            strncpy(aAddrInfo->mSockUnion.sun.sun_path,
                aAddrInfo->mAddr.c_str(),
                sizeof(aAddrInfo->mSockUnion.sun.sun_path)-1);
            break;
        default:
            LOG(LL_ERROR, "unknown AF family %d.", aAddrInfo->mFamily);
            return RET_ERROR;
    }
    DEBUG(LL_ALL, "End.");
    return RET_OK;
}


// get port of ipv6 and ipv4.
unsigned short Socket::SuGetPort(union sockaddr_union* su) {
    switch (su->s.sa_family) {
        case AF_INET:
            return ntohs(su->sin.sin_port);
        case AF_INET6:
            return ntohs(su->sin6.sin6_port);
        default:
            return 0;
    }
}

// ---------------------------------------------------------------------------
// sets the port number (host byte order)
// ---------------------------------------------------------------------------
//
void Socket::SuSetPort(union sockaddr_union* su, unsigned short port) {
    switch (su->s.sa_family) {
    case AF_INET:
        su->sin.sin_port=htons(port);
        break;
    case AF_INET6:
        su->sin6.sin6_port=htons(port);
        break;
    default:
        LOG(LL_ERROR, "su_set_port: BUG: unknown address family %d\n",
        su->s.sa_family);
    }
}

// ---------------------------------------------------------------------------
// size of sockaddr_union
// ---------------------------------------------------------------------------
//
int Socket::SuSize(const sockaddr_union& su) {
    return su.s.sa_family == PF_INET6 ?
        sizeof(struct sockaddr_in6) :
        sizeof(struct sockaddr_in);
}

// ---------------------------------------------------------------------------
// create socket,socket.
// ---------------------------------------------------------------------------
//
int Socket::CreateSocket(SAddrInfo* aAddrInfo) {
    if (aAddrInfo == NULL) return RET_ERROR;
    DEBUG(LL_ALL, "Begin.");
    if (aAddrInfo->mFamily != AF_INET && aAddrInfo->mFamily != AF_INET6) {
        aAddrInfo->mFamily = AF_INET;
    }
    if (InitSockAddrUn(aAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, "InitSockAddrUn client error.");
        return RET_ERROR;
    }
    LOG(LL_DBG, "try to make fd by socket method.");
    if ((aAddrInfo->mSockId = socket(aAddrInfo->mSockUnion.s.sa_family,
        PROTOCOL(aAddrInfo->mProtocol), 0)) < 0) {
        LOG(LL_ERROR, "socket error.");
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End.");
    return RET_OK;
}

// ---------------------------------------------------------------------------
// setsockopt,bind and listen.
// ---------------------------------------------------------------------------
//
int Socket::ListenServer(SAddrInfo* aAddrInfo) {
    if (aAddrInfo == NULL) return RET_ERROR;
    DEBUG(LL_ALL, "Begin.");
    int ret = RET_OK;
    do {
        if (SetReuseAddr(aAddrInfo->mSockId) == RET_ERROR) {
            LOG(LL_ERROR, "setsockopt SO_REUSEADDR or SO_REUSEPORT error.");
            ret = RET_ERROR;
            break;
        }
        LOG(LL_DBG, "try to bind socket.");
        if (bind(aAddrInfo->mSockId,
            &aAddrInfo->mSockUnion.s,
            SuSize(aAddrInfo->mSockUnion)) < 0) {
            LOG(LL_ERROR, "bind error.");
            ret = RET_ERROR;
            break;
        }
        if (aAddrInfo->mProtocol == TCP) {
            LOG(LL_DBG, "try to listen socket.");
            if (listen(aAddrInfo->mSockId, 511) < 0) {
                LOG(LL_ERROR, "listen error.");
                ret = RET_ERROR;
                break;
            }
            if (SetLinger(aAddrInfo->mSockId) == RET_ERROR) {
                LOG(LL_ERROR, "SetLinger error.");
                ret = RET_ERROR;
                break;
            }
        }
    } while (0);
    if (ret == RET_ERROR) {
        close(aAddrInfo->mSockId);
    }
    LOG(LL_VARS, "socket id:[%d]", aAddrInfo->mSockId);
    return ret;
}

int Socket::TcpServer(SAddrInfo* aAddrInfo) {
    DEBUG(LL_ALL, "Begin.");
    if (aAddrInfo == NULL) {
        LOG(LL_ERROR, "address is null.");
        return RET_ERROR;
    }
    if (CreateSocket(aAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, ":create Socket.socket failed.");
        return RET_ERROR;
    }
    if (ListenServer(aAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, ":ListenServer.bind.listen failed.");
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End.");
    return RET_OK;
}

int Socket::Tcp6Server(SAddrInfo* aAddrInfo) {
    DEBUG(LL_ALL, "Begin.");
    if (aAddrInfo == NULL) {
        LOG(LL_ERROR, "address is null.");
        return RET_ERROR;
    }
    aAddrInfo->mFamily = AF_INET6;
    if (CreateSocket(aAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, ":create Socket.socket failed.");
        return RET_ERROR;
    }
    if (ListenServer(aAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, ":ListenServer.bind.listen failed.");
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End.");
    return RET_OK;
}

int Socket::TcpUnixServer(SAddrInfo* aAddrInfo) {
    DEBUG(LL_ALL, "Begin.");
    if (aAddrInfo == NULL) {
        LOG(LL_ERROR, "address is null.");
        return RET_ERROR;
    }
    if ((aAddrInfo->mSockId = socket(AF_LOCAL,
        PROTOCOL(aAddrInfo->mProtocol), 0)) < 0) {
        LOG(LL_ERROR, "socket error.");
        return RET_ERROR;
    }
    aAddrInfo->mSockUnion.sun.sun_family = AF_LOCAL;
    strncpy(aAddrInfo->mSockUnion.sun.sun_path,
        aAddrInfo->mAddr.c_str(), sizeof(aAddrInfo->mSockUnion.sun.sun_path)-1);
    if (ListenServer(aAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, ":ListenServer.bind.listen failed.");
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End.");
    return RET_OK;
}
// ---------------------------------------------------------------------------
// connect.
// ---------------------------------------------------------------------------
//
int Socket::TcpConnect(const SAddrInfo& aAddrInfo) {
    LOG(LL_DBG, "try to connect sockid:(%d).", aAddrInfo.mSockId);
    int ret = connect(aAddrInfo.mSockId, &(aAddrInfo.mSockUnion.s),
        SuSize(aAddrInfo.mSockUnion)) < 0;
    // EINPROGRESS for nonblock.
    if (ret != 0 && errno != EINPROGRESS) {
        close(aAddrInfo.mSockId);
        LOG(LL_ERROR, "connect sockid:(%d),error:(%s).",
            aAddrInfo.mSockId, strerror(errno));
        return RET_ERROR;
    }
    LOG(LL_DBG, "connect sockid:(%d) success.", aAddrInfo.mSockId);
    return RET_OK;
}

// close the socket by fd
void Socket::CloseSocket(int aSockId) {
    if (aSockId > 0) {
        close(aSockId);
    }
}

// shutdown sockid
void Socket::ShutdownWrite(int aSockId) {
    if (::shutdown(aSockId, SHUT_WR) < 0) {
        LOG(LL_ERROR, "shutdown sockid:(%d),error:(%s).",
            aSockId, strerror(errno));
    }
}
// ---------------------------------------------------------------------------
// make connect to tcp server.
// ---------------------------------------------------------------------------
//
int Socket::MakeTcpConn(SAddrInfo* aAddrInfo, bool aIsBlock /* = true */) {
    DEBUG(LL_ALL, "Begin");
    if (aAddrInfo == NULL) {
        LOG(LL_ERROR, "address is null.");
        return RET_ERROR;
    }
    DEBUG(LL_DBG, "try to create socket.");
    if (RET_ERROR == CreateSocket(aAddrInfo)) {
        LOG(LL_ERROR, "CreateSocket.ip:(%s),port:(%d),error:(%s).",
            aAddrInfo->mAddr.c_str(), aAddrInfo->mPort, strerror(errno));
        return RET_ERROR;
    }
    // set non-block socket.
    if (!aIsBlock) {
        setNonBlock(aAddrInfo->mSockId, true);        
    }
    
    int tryTime = 0, ret = RET_ERROR;
    do {
        DEBUG(LL_DBG, "try to connect ip:(%s), port:(%d), trytime:(%d).",
            aAddrInfo->mAddr.c_str(), aAddrInfo->mPort, tryTime);
        ret = TcpConnect(*aAddrInfo);
    } while (ret == RET_ERROR && tryTime++ < 3);
    if (RET_ERROR == ret) {
        LOG(LL_ERROR, "TcpConnect.ip:(%s),port:(%d),error:(%s).",
            aAddrInfo->mAddr.c_str(), aAddrInfo->mPort, strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

int Socket::MakeUnixConnect(SAddrInfo* aAddrInfo) {
    DEBUG(LL_ALL, "Begin");
    if (aAddrInfo == NULL) {
        LOG(LL_ERROR, "address is null.");
        return RET_ERROR;
    }
    aAddrInfo->mFamily = AF_LOCAL;
    DEBUG(LL_DBG, "try to create socket.");
    if (RET_ERROR == CreateSocket(aAddrInfo)) {
        LOG(LL_ERROR, "CreateSocket.unix_path:(%s),port:(%d),error:(%s).",
            aAddrInfo->mAddr.c_str(), aAddrInfo->mPort, strerror(errno));
        return RET_ERROR;
    }
    int tryTime = 0, ret = RET_ERROR;
    do {
        DEBUG(LL_DBG, "try to connect ip:(%s), port:(%d), trytime:(%d).",
            aAddrInfo->mAddr.c_str(), aAddrInfo->mPort, tryTime);
        ret = TcpConnect(*aAddrInfo);
    } while (ret == RET_ERROR && tryTime++ < 3);
    if (RET_ERROR == ret) {
        LOG(LL_ERROR, "TcpConnect.ip:(%s),port:(%d),error:(%s).",
            aAddrInfo->mAddr.c_str(), aAddrInfo->mPort, strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

// ---------------------------------------------------------------------------
// accept socket, the client is added into host queue when new conn is coming.
// ---------------------------------------------------------------------------
//
int Socket::AcceptConnection(int aSockId, SAddrInfo* aClntAddrInfo) {
    DEBUG(LL_ALL, "Begin.");
    int clntSock = RET_ERROR;

#ifdef HPUX
    int clntlen;
#else
    socklen_t clntlen;
#endif
    clntlen = sizeof(aClntAddrInfo->mSockUnion);
    do {
        clntSock = accept(aSockId, &(aClntAddrInfo->mSockUnion.s), &clntlen);
    } while (clntSock < 0 && errno == EINTR);
    if (clntSock < 0) {
        LOG(LL_ERROR, "Accept socket fail, sockid:(%d),error:(%s)",
            aSockId, strerror(errno));
        return RET_ERROR;
    }
    SAddrInfo2String(aClntAddrInfo);
    aClntAddrInfo->mSockId = clntSock;
    LOG(LL_WARN, "new client ip:(%s),port:(%d),sockid:(%d)",
        aClntAddrInfo->mAddr.c_str(),
        aClntAddrInfo->mPort, aClntAddrInfo->mSockId);
    DEBUG(LL_ALL, "End.");
    return RET_OK;
}

// ---------------------------------------------------------------------------
// SAddrInfo to ip:port string:int.
// ---------------------------------------------------------------------------
//
int Socket::SAddrInfo2String(SAddrInfo* aClntAddrInfo) {
    if (aClntAddrInfo == NULL) return RET_ERROR;
    DEBUG(LL_ALL, "Begin.");
    char clnIpBuf[64] = {0};
    if (aClntAddrInfo->mSockUnion.s.sa_family == AF_INET) {
        inet_ntop(AF_INET, &(aClntAddrInfo->mSockUnion.sin.sin_addr),
            clnIpBuf, sizeof(clnIpBuf));
        aClntAddrInfo->mPort = ntohs(aClntAddrInfo->mSockUnion.sin.sin_port);
    } else {
        inet_ntop(AF_INET6, &(aClntAddrInfo->mSockUnion.sin6.sin6_addr),
            clnIpBuf, sizeof(clnIpBuf));
        aClntAddrInfo->mPort = ntohs(aClntAddrInfo->mSockUnion.sin6.sin6_port);
    }
    aClntAddrInfo->mAddr = clnIpBuf;
    DEBUG(LL_ALL, "End.");
    return RET_OK;
}

// host name is ip address to ipv4 or ipv6 if aIsIPOnly is true.
// otherwise, host name is normal string instead of ip address format.
bool Socket::HostResolve(const char* aInHost, char *aOutIP, int aOutSize, bool aIsIPOnly) {
    if (aInHost == NULL || aOutIP == NULL) return false;
    struct addrinfo hints, *info;
    int rv;

    memset(&hints, 0, sizeof(hints));
    if (aIsIPOnly) {
        //IP_ONLY
        hints.ai_flags = AI_NUMERICHOST;
    }
    hints.ai_family = AF_UNSPEC;
    // specify socktype to avoid dups
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(aInHost, NULL, &hints, &info)) != 0) {
        LOG(LL_ERROR, "getaddrinfo fail error:(%s)", gai_strerror(rv));
        return false;
    }
    if (info->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), aOutIP, aOutSize);
    } else {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)info->ai_addr;
        inet_ntop(AF_INET6, &(sa->sin6_addr), aOutIP, aOutSize);
    }

    freeaddrinfo(info);
    return true;
}

// ---------------------------------------------------------------------------
// DNS to ip address.
// ---------------------------------------------------------------------------
//
bool Socket::Name2IPv4(const char* aInHost, char *aOutIPv4) {
    if ((int)(inet_addr(aInHost)) == -1) {
        struct hostent *hp;
        struct in_addr in;
        hp = gethostbyname(aInHost);
        if (hp == NULL) {
            return false;
        }
        memcpy(&in.s_addr, *(hp->h_addr_list), sizeof(in.s_addr));
        strcpy(aOutIPv4, inet_ntoa(in));
    }
    return true;
}

// whether ipstr with ipv4 format x.x.x.x.
bool Socket::IsIPv4Addr(const string &aIpStr) {
    struct in_addr addr;
    if (inet_aton(aIpStr.c_str(), &addr) != 0)
        return true;
    else
        return false;
}

// whether ipstr with ipv6 format
bool Socket::IsIPv6Addr(const string &aIpStr) {
    struct in6_addr addr;
    if (inet_pton(AF_INET6, aIpStr.c_str(), &addr) != 0)
        return true;
    else
        return false;
}

// ---------------------------------------------------------------------------
// get remote IP and Port by socket id.
// ---------------------------------------------------------------------------
//
int Socket::NetPeerToStr(int fd, char *aIp, size_t aIpLen, int *aPort) {
    struct sockaddr_storage sa;
    #ifdef HPUX
    int salen = sizeof(sa);
    #else
    socklen_t salen = sizeof(sa);
    #endif

    if (getpeername(fd, (struct sockaddr*)&sa, &salen) == -1) {
        if (aPort) *aPort = 0;
        aIp[0] = '?';
        aIp[1] = '\0';
        return RET_ERROR;
    }
    if (aPort) *aPort = 0;
    if (sa.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        if (aIp) inet_ntop(AF_INET, (void*)&(s->sin_addr), aIp, aIpLen);
        if (aPort) *aPort = ntohs(s->sin_port);
    } else if (sa.ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        if (aIp) inet_ntop(AF_INET6, (void*)&(s->sin6_addr), aIp, aIpLen);
        if (aPort) *aPort = ntohs(s->sin6_port);
    } else if (sa.ss_family == AF_UNIX) {
        if (aIp) strncpy(aIp, "/unixsocket", aIpLen);
    } else {
        aIp[0] = '?';
        aIp[1] = '\0';
        return RET_ERROR;
    }
    return RET_OK;
}

// ---------------------------------------------------------------------------
// get local IP and Port by socket id.
// ---------------------------------------------------------------------------
//
int Socket::NetLocalSockName(int aSockId, char *aIp, size_t aIpLen, int *aPort) {
    struct sockaddr_storage sa;
    #ifdef HPUX
    int salen = sizeof(sa);
    #else
    socklen_t salen = sizeof(sa);
    #endif

    if (getsockname(aSockId, (struct sockaddr*)&sa, &salen) == -1) {
        if (aPort) *aPort = 0;
        aIp[0] = '?';
        aIp[1] = '\0';
        return RET_ERROR;
    }
    if (sa.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        if (aIp) inet_ntop(AF_INET, (void*)&(s->sin_addr), aIp, aIpLen);
        if (aPort) *aPort = ntohs(s->sin_port);
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        if (aIp) inet_ntop(AF_INET6, (void*)&(s->sin6_addr), aIp, aIpLen);
        if (aPort) *aPort = ntohs(s->sin6_port);
    }
    return RET_OK;
}

// ---------------------------------------------------------------------------
// fcntl socket id to NO_BLOCK.
// ---------------------------------------------------------------------------
//
int Socket::setNonBlock(int aSockId, bool aIssetNonBlock) {
    DEBUG(LL_ALL, "Begin");
    /* Set the socket non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    int flags;
    if ((flags = fcntl(aSockId, F_GETFL)) == RET_ERROR) {
        LOG(LL_ERROR, "fcntl(F_GETFL): %s", strerror(errno));
        return RET_ERROR;
    }
    if (aIssetNonBlock) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    if (fcntl(aSockId, F_SETFL, flags) == RET_ERROR) {
        LOG(LL_ERROR, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

/* Set TCP keep alive option to detect dead peers. The interval option
 * is only used for Linux as we are using Linux-specific APIs to set
 * the probe send time, interval, and count. */
int Socket::KeepAlive(int aSockId, int aInterval) {
    DEBUG(LL_ALL, "End");
    LOG(LL_VARS, "sockid:(%d), interval:(%d)", aSockId, aInterval);
    int val = 1;
    if (setsockopt(aSockId, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1) {
        LOG(LL_ERROR, "setsockopt SO_KEEPALIVE: error:(%s).", strerror(errno));
        return RET_ERROR;
    }

#ifdef __linux__
    /* Default settings are more or less garbage, with the keepalive time
     * set to 7200 by default on Linux. Modify settings to make the feature
     * actually useful. */

    /* Send first probe after interval. */
    val = aInterval;
    if (setsockopt(aSockId, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
        LOG(LL_ERROR, "setsockopt TCP_KEEPIDLE: error:(%s).", strerror(errno));
        return RET_ERROR;
    }

    /* Send next probes after the specified interval. Note that we set the
     * delay as interval / 3, as we send three probes before detecting
     * an error (see the next setsockopt call). */
    val = aInterval/3;
    if (val == 0) val = 1;
    if (setsockopt(aSockId, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
        LOG(LL_ERROR, "setsockopt TCP_KEEPINTVL: error:(%s).", strerror(errno));
        return RET_ERROR;
    }

    /* Consider the socket in error state after three we send three ACK
     * probes without getting a reply. */
    val = 3;
    if (setsockopt(aSockId, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
        LOG(LL_ERROR, "setsockopt TCP_KEEPCNT: error:(%s).", strerror(errno));
        return RET_ERROR;
    }
#endif
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

// ---------------------------------------------------------------------------
// set tcp socket no delay.
// aVal=1:enable, aVal=1:disable
// ---------------------------------------------------------------------------
//
int Socket::SetTcpNoDelay(int aSockId, int aVal) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), val:(%d)", aSockId, aVal);
    if (setsockopt(aSockId, IPPROTO_TCP, TCP_NODELAY,
        &aVal, sizeof(aVal)) == -1) {
        LOG(LL_ERROR, "setsockopt TCP_NODELAY: error:(%s).", strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

bool Socket::setTcpQuickAck(int fd, bool quickAck) {
    bool rc = false;
    int quickAckInt = quickAck ? 1 : 0;
    if (fd > 0) {
        rc = (setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK,
            (const void *)(&quickAckInt), sizeof(quickAckInt)) == 0);
    }
    return rc;
}

// ---------------------------------------------------------------------------
// set socket int type option.
// ---------------------------------------------------------------------------
//
bool Socket::setIntOption(int fd, int option, int value) {
    bool rc = false;
    if (fd > 0) {
        rc = (::setsockopt(fd, SOL_SOCKET, option,
            (const void *)(&value), sizeof(value)) == 0);
    }
    return rc;
}

// ---------------------------------------------------------------------------
// set socket send buffer size.
// ---------------------------------------------------------------------------
//
int Socket::SetSendBuffer(int aSockId, int aBuffSize) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), buffersize:(%d)", aSockId, aBuffSize);
    if (setsockopt(aSockId, SOL_SOCKET, SO_SNDBUF,
        &aBuffSize, sizeof(aBuffSize)) == -1) {
        LOG(LL_ERROR, "setsockopt SO_SNDBUF: error:(%s).", strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

// ---------------------------------------------------------------------------
// set socket recv buffer size.
// ---------------------------------------------------------------------------
//
int Socket::SetRecvBuffer(int aSockId, int aBuffSize) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), buffersize:(%d)", aSockId, aBuffSize);
    if (setsockopt(aSockId, SOL_SOCKET, SO_RCVBUF,
        &aBuffSize, sizeof(aBuffSize)) == -1) {
        LOG(LL_ERROR, "setsockopt SO_RCVBUF: error:(%s).", strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

int Socket::SetBroadCast(int aSockId, int aBroadCast) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), broad cast flag:(%d)", aSockId, aBroadCast);
    if (setsockopt(aSockId, SOL_SOCKET, SO_BROADCAST,
        &aBroadCast, sizeof(aBroadCast)) < 0) {
        LOG(LL_ERROR, "setsockopt SO_BROADCAST: error:(%s).", strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

// ---------------------------------------------------------------------------
// aFlag = 1:add into group. aFalg=0:drop from group.
// ---------------------------------------------------------------------------
//
int Socket::SetMultiCast(int aSockId, EMultiCaseOption aFlag,
                          const string& aMultiAddress) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), multi cast flag:(%d), address:(%s)",
        aSockId, aFlag, aMultiAddress.c_str());
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(aMultiAddress.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (aFlag == kJoinMultiCast) {
        // add into group.
        if(setsockopt(aSockId, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            &mreq, sizeof(mreq)) < 0) {
            LOG(LL_ERROR, "setsockopt IP_ADD_MEMBERSHIP: error:(%s).",
                strerror(errno));
            return RET_ERROR;
        }
    } else {
        // drop membership.
        if(setsockopt(aSockId, IPPROTO_IP, IP_DROP_MEMBERSHIP,
            &mreq, sizeof(mreq)) < 0) {
            LOG(LL_ERROR, "setsockopt IP_DROP_MEMBERSHIP: error:(%s).",
                strerror(errno));
            return RET_ERROR;
        }
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

// aFlag = 1:add into group. aFalg=0:drop from group.
// aIfName is interface name, such as 'etho'.
int Socket::SetMultiCastIpv6(int aSockId, EMultiCaseOption aFlag,
    const struct sockaddr_in6 *aSin6, const char *aIfName) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d), multi cast flag:(%d)", aSockId, aFlag);
    struct ipv6_mreq mreq;
    if (aIfName != NULL && aIfName[0] != 0x00) {
        if ((mreq.ipv6mr_interface = if_nametoindex(aIfName)) == 0) {
            errno = ENXIO;
            return RET_ERROR;
        }
    } else {
        mreq.ipv6mr_interface = 0;
    }
    memcpy(&mreq.ipv6mr_multiaddr, &aSin6->sin6_addr, sizeof(struct in6_addr));
    if (aFlag == kJoinMultiCast) {
        // add into group.
        if(setsockopt(aSockId, IPPROTO_IPV6, IPV6_JOIN_GROUP,
            &mreq, sizeof(mreq)) < 0) {
            LOG(LL_ERROR, "setsockopt IPV6_JOIN_GROUP: error:(%s).",
                strerror(errno));
            return RET_ERROR;
        }
    } else {
        // drop membership.
        if(setsockopt(aSockId, IPPROTO_IPV6, IPV6_LEAVE_GROUP,
            &mreq, sizeof(mreq)) < 0) {
            LOG(LL_ERROR, "setsockopt IPV6_LEAVE_GROUP: error:(%s).",
                strerror(errno));
            return RET_ERROR;
        }
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

// SO_REUSEADDR or SO_REUSEPORT is set for TCP
int Socket::SetReuseAddr(int aSockId) {
    int opt = 1;
#if defined(HPUX) || defined(AIX)
    if (setsockopt(aSockId, SOL_SOCKET, SO_REUSEPORT,
        (char *)&opt,sizeof(opt)) < 0)
#else
    if (setsockopt(aSockId, SOL_SOCKET, SO_REUSEADDR,
        (char *)&opt,sizeof(opt)) < 0)
#endif
    {
        return RET_ERROR;
    }
    return RET_OK;
}
// ---------------------------------------------------------------------------
// SO_LINGER is set for TCP.
// ---------------------------------------------------------------------------
//
int Socket::SetLinger(int aSockId) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d)", aSockId);
    struct linger l;
    l.l_onoff=1;
    l.l_linger=0;
    if (setsockopt(aSockId, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof(l)) < 0) {
        LOG(LL_ERROR, "setsockopt SO_LINGER error:(%s).", strerror(errno));
        close(aSockId);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

int Socket::SetV6Only(int aSockId) {
    DEBUG(LL_ALL, "Begin");
    DEBUG(LL_VARS, "sockid:(%d)", aSockId);
    int yes = 1;
    if (setsockopt(aSockId, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) < 0) {
        LOG(LL_ERROR, "setsockopt SO_LINGER error:(%s).", strerror(errno));
        close(aSockId);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}

// ---------------------------------------------------------------------------
// SO_RCVTIMEO and SO_SNDTIMEO are set for UDP.
// ---------------------------------------------------------------------------
//
int Socket::SetSockTimeOut(int aSockId, int aFlag, int aTimeOut) {
    DEBUG(LL_ALL, "Begin");
    LOG(LL_VARS, "sockid:(%d),flag:(%d),timeout:(%d)",
        aSockId, aFlag, aTimeOut);
    if (aTimeOut < 0) {
        aTimeOut = FD_WAIT_MS;
    }

    struct timeval tv_out;
    tv_out.tv_sec = aTimeOut/1000;
    tv_out.tv_usec = (aTimeOut % 1000) * 1000;
    if(tv_out.tv_usec >= 1000000) {
        tv_out.tv_sec++;
        tv_out.tv_usec -= 1000000;
    }
    if (aFlag & FD_SET_SO_RCVTIMEO) {
        // setsockopt(aSockId, SOL_SOCKET, SO_RCVTIMEO, &aTimeOut, sizeof(aTimeOut)
        if (setsockopt(aSockId, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out)) < 0) {
            LOG(LL_ERROR, "setsockopt SO_RCVTIMEO error:(%s).", strerror(errno));
            close(aSockId);
            return RET_ERROR;
        }
    }
    if (aFlag & FD_SET_SO_SNDTIMEO) {
        if (setsockopt(aSockId, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out)) < 0) {
            LOG(LL_ERROR, "setsockopt SO_SNDTIMEO error:(%s).", strerror(errno));
            close(aSockId);
            return RET_ERROR;
        }
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}


// ---------------------------------------------------------------------------
// get socket error
// ---------------------------------------------------------------------------
//
int Socket::getSoError(int fd) {
    if (fd < 0) {
        return EINVAL;
    }

    int lastError = errno;
    int  soError = 0;
    socklen_t soErrorLen = sizeof(soError);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR,
        (void *)(&soError), &soErrorLen) != 0) {
        return lastError;
    }
    if (soErrorLen != sizeof(soError))
        return EINVAL;

    return soError;
}

// ---------------------------------------------------------------------------
// Like read(2) but make sure 'aCount' is read before to return
// (unless error or EOF condition is encountered)
// ---------------------------------------------------------------------------
//
int Socket::NetRead(int aSockId, char *aBuf, int aCount) {
    if (aBuf == NULL) return RET_ERROR;
    int nread =0 , totlen = 0;
    while (totlen != aCount) {
        nread = read(aSockId, aBuf+totlen, aCount-totlen);
        if (nread == 0) return totlen;
        if (nread < 0 && errno == EINTR) continue;
        if (nread == -1) return RET_ERROR;
        totlen += nread;
    }
    aBuf[totlen] = 0x00;
    return totlen;
}

// ---------------------------------------------------------------------------
// int Socket::NetWrite(int aSockId, char *aBuf, int aCount)
//
// Like write(2) but make sure 'aCount' is read before to return
// (unless error is encountered)
// ---------------------------------------------------------------------------
//
int Socket::NetWrite(int aSockId, const char* aBuf, int aCount) {
    if (aBuf == NULL) return RET_ERROR;
    int nwritten =0, totlen = 0;
    while (totlen != aCount) {
        nwritten = write(aSockId, aBuf+totlen, aCount-totlen);
        if (nwritten == 0) return totlen;
        if (nwritten < 0 && errno == EINTR) continue;
        if (nwritten == -1) return RET_ERROR;
        totlen += nwritten;
    }
    return totlen;
}

// ---------------------------------------------------------------------------
// check socket id whether event is occurred.
// ---------------------------------------------------------------------------
//
int Socket::SelectSockId(int aSockId, int aTimeOut, int aMask) {
    DEBUG(LL_ALL, "Begin");
    if (aSockId < 0 || aMask == FD_NONE) {
        LOG(LL_ERROR, "bad socket id:(%d).", aSockId);
        return RET_ERROR;
    }
    if (aTimeOut < 0) aTimeOut = FD_WAIT_MS;

    DEBUG(LL_VARS, "sockid:(%d),timeout:(%d),flag:(%d)",
        aSockId, aTimeOut, aMask);

    //init result check
    bool bRead  = (aMask & FD_READABLE) ? true : false;
    bool bWrite = (aMask & FD_WRITABLE) ? true : false;
    bool bExcep = (aMask & FD_RWERROR)  ? true : false;

    struct timeval tval, *tvalp;
    fd_set rset, wset, eset;
    fd_set *rsetptr = NULL;
    fd_set *wsetptr = NULL;
    fd_set *esetptr = NULL;
    // using tick hz is instead of gettimebyday when EINTR == errno.
    unsigned int TimeOutHz = aTimeOut / 16;
    while (true) {
        // set select timeout
        tvalp = NULL;
        if (aTimeOut > 0) {
            tval.tv_sec = aTimeOut/1000;
            tval.tv_usec = (aTimeOut % 1000) * 1000;
            if(tval.tv_usec >= 1000000) {
                tval.tv_sec++;
                tval.tv_usec -= 1000000;
            }
            tvalp = &tval;
        } else {
            LOG(LL_ERROR, "Socket time out is occurred.");
            return RET_ERROR;
        }
        // reset the fd set pointer
        rsetptr = NULL;
        wsetptr = NULL;
        esetptr = NULL;

        if (bRead) {
            FD_ZERO(&rset);
            FD_SET(aSockId, &rset);
            rsetptr = &rset;
        }
        if (bWrite) {
            FD_ZERO(&wset);
            FD_SET(aSockId, &wset);
            wsetptr = &wset;
        }
        if (bExcep) {
            FD_ZERO(&eset);
            FD_SET(aSockId, &eset);
            esetptr = &eset;
        }
        int ret = select(aSockId+1, rsetptr, wsetptr, esetptr, tvalp);
        if (0 == ret) {
            // timeout, result is set to 0
            LOG(LL_ERROR, "select time out error.");
            return RET_ERROR;
        } else if(0 > ret) {
            // check errno
            if(EINTR == errno) {
                LOG(LL_WARN, "EINTR is occurred.");
                aTimeOut -= TimeOutHz;
                continue;
            }
            LOG(LL_ERROR, "select:(%d) other error:(%s)",
                ret, strerror(errno));
            // error ocurred, but not EINTR
            return RET_ERROR;
        } // end if(0 == ret)

        // select ok, break the loop
        break;
    } // end while

    DEBUG(LL_INFO, "select returned.");
    //clear the result
    int result = 0;

    //check the read fd set
    if (bRead && FD_ISSET(aSockId, &rset)) {
        //select read ok
        DEBUG(LL_INFO, "select read set is ok.");
        result |= FD_READABLE;
    }

    //check the write fd set
    if (bWrite && FD_ISSET(aSockId, &wset)) {
        //select write ok
        DEBUG(LL_INFO, "select write set is ok.");
        result |= FD_WRITABLE;
    }

    //check the exception fd set
    if (bExcep && FD_ISSET(aSockId, &eset)) {
        //select exception ok
        DEBUG(LL_INFO, "select exception set is ok.");
        result |= FD_RWERROR;
    }
    DEBUG(LL_INFO, "return result=%d", result);
    return result;
}

// Wait for milliseconds until the given file descriptor becomes
// writable/readable/exception
int Socket::PollSockId(int aSockId, long long aMillisecs, int aMask) {
    struct pollfd pfd;
    int retmask = 0, retval;

    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = aSockId;
    if (aMask & FD_READABLE) pfd.events |= POLLIN;
    if (aMask & FD_WRITABLE) pfd.events |= POLLOUT;

    if ((retval = poll(&pfd, 1, aMillisecs))== 1) {
        if (pfd.revents & POLLIN) retmask |= FD_READABLE;
        if (pfd.revents & POLLOUT) retmask |= FD_WRITABLE;
        if (pfd.revents & POLLERR) retmask |= FD_WRITABLE;
        if (pfd.revents & POLLHUP) retmask |= FD_WRITABLE;
        return retmask;
    } else {
        return retval;
    }
}

#define TSEND_RECV_INIT(ev) \
    int n = 0; \
struct pollfd pf; \
    unsigned int expire, ticks_raw; \
    signed short diff; \
    short event; \
    ticks_raw = TIMER_TICKS_HZ; \
    expire = ticks_raw + MS_TO_TICKS((unsigned int)timeout); \
    event = ev; \
    pf.fd=fd; \
    pf.events=event

#define TSEND_RECV_POLL(f_name) \
poll_loop: \
    while(1){ \
        if (timeout==-1) \
            n=poll(&pf, 1, -1); \
        else{ \
            diff=expire-ticks_raw; \
            if (diff<=0){ \
                LOG(LL_ERROR, "ERROR: " f_name ": timeout (%d)\n", timeout);\
                goto error; \
            } \
            n=poll(&pf, 1, TICKS_TO_MS((unsigned int)diff)); \
        } \
        if (n<0){ \
            if (errno==EINTR) continue; \
                LOG(LL_ERROR, "ERROR: " f_name ": poll failed: %s [%d]\n", \
                    strerror(errno), errno); \
            goto error; \
        } else if (n==0){ \
            LOG(LL_ERROR, "ERROR: " f_name ": timeout (p %d)\n", timeout); \
            goto error; \
        } \
        if (pf.revents&event){ \
            goto again; \
        } else if (pf.revents&(POLLERR|POLLHUP|POLLNVAL)){ \
            LOG(LL_ERROR, "ERROR: " f_name ": bad poll flags %x\n", \
            pf.revents); \
            goto error; \
        } \
    }

#define TSEND_RECV_ERR_CHECK(f_name)\
    if (n<0){ \
        if (errno==EINTR) goto again; \
        else if (errno!=EAGAIN && errno!=EWOULDBLOCK){ \
            LOG(LL_ERROR, "ERROR: " f_name ": failed: (%d) %s\n", \
                errno, strerror(errno)); \
            goto error; \
        } else goto poll_loop; \
    }

int Socket::UdpSend(int fd, char* buffer, int b_size,
                     int timeout, union sockaddr_union* su_cliaddr) {
    if (buffer == NULL || b_size < 0 || su_cliaddr == NULL) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.", b_size);
        return RET_ERROR;
    }
    DEBUG_HEX_ASC(LL_VARS, buffer, b_size);
    TSEND_RECV_INIT(POLLOUT);
again:
    n = sendto(fd, buffer, b_size, 0,
        &((*su_cliaddr).s), sizeof(*su_cliaddr));
    TSEND_RECV_ERR_CHECK("UdpSend");
    return n;
    TSEND_RECV_POLL("UdpSend");
error:
    LOG(LL_ERROR, "while sending packet: %s", strerror(errno));
    return RET_ERROR;
}

int Socket::UdpRecv(int fd, char *pkg_buffer, int b_size,
                     int timeout, union sockaddr_union* su_cliaddr) {
    if (pkg_buffer == NULL || b_size < 0 || su_cliaddr == NULL) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.", b_size);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "Begin");
#ifdef HPUX
    int clntlen;
#else
    socklen_t clntlen;
#endif
    TSEND_RECV_INIT(POLLIN);
    clntlen = sizeof(*su_cliaddr);
again:
    n = recvfrom(fd, pkg_buffer, b_size, 0,
        &((*su_cliaddr).s), &clntlen);

    LOG(LL_VARS, "recvfrom str:");
    LOG_HEX_ASC(LL_VARS, pkg_buffer, n);
    TSEND_RECV_ERR_CHECK("UdpRecv");
    DEBUG(LL_ALL, "End");
    return n;
    TSEND_RECV_POLL("UdpRecv");
error:
    LOG(LL_ERROR, "while recving packet error:(%s).", strerror(errno));
    return RET_ERROR;
}

// ---------------------------------------------------------------------------
// send buffer.
// ---------------------------------------------------------------------------
//
int Socket::TcpSend(int aSockId, const char* aBuffer,
                     int aBufSize, int aTimeOut, bool aHasPolled) {
    DEBUG(LL_ALL, "Begin");
    if (aBuffer == NULL || aBufSize < 0) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.", aBufSize);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "sockid:(%d), sendsize:(%d), timeout:(%d)",
        aSockId, aBufSize, aTimeOut);
    LOG_HEX_ASC(LL_VARS, aBuffer, aBufSize);
    int n = 0, sendSize = 0, status = FD_WRITABLE;
    // aTimeOut *= TIMER_S_TO_MS;
    // using tick hz is instead of time(NULL) when write.
    unsigned int tmpTimeOutHz = aTimeOut / TIMER_TICKS_HZ;
    while (aBufSize > 0) {
        aTimeOut -= tmpTimeOutHz;
        if (aTimeOut <= 0) {
            LOG(LL_ERROR, "Socket time out is occurred.");
            return RET_ERROR;
        }
        if (aHasPolled) {
            status = FD_WRITABLE;
        } else {
            status = PollSockId(aSockId, aTimeOut, FD_WRITABLE|FD_RWERROR);
            //status = SelectSockId(aSockId, aTimeOut, FD_WRITABLE|FD_RWERROR);
            if (status <= 0 || status & FD_RWERROR) {
                LOG(LL_ERROR, "SelectSockId error status:(%d).", status);
                return RET_ERROR;
            }
        }
        if (status & FD_WRITABLE) {
            if ((n = write(aSockId, aBuffer+sendSize, aBufSize)) <= 0) {
                LOG(LL_ERROR, "write packet error:(%s)", strerror(errno));
                return RET_ERROR;
            }
            aBufSize -= n;
            sendSize += n;
        }
        aHasPolled = false;
    }
    DEBUG(LL_ALL, "End");
    return sendSize;
}

// ---------------------------------------------------------------------------
// send buffer.
// ---------------------------------------------------------------------------
//
int Socket::TcpSendV(int aSockId, const struct iovec* aIovec,
                      int aIovCnt, int aTimeOut, bool aHasPolled) {
    DEBUG(LL_ALL, "Begin");
    if (aIovec == NULL || aIovCnt <= 0) {
        LOG(LL_ERROR, "vector is null or size:(%d) less than zero.", aIovCnt);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "sockid:(%d), count:(%d), timeout:(%d)",
        aSockId, aIovCnt, aTimeOut);
    int n = 0, status = FD_WRITABLE;
    // aTimeOut *= TIMER_S_TO_MS;
    // using tick hz is instead of time(NULL) when write.
    unsigned int tmpTimeOutHz = aTimeOut / TIMER_TICKS_HZ;
    while (aTimeOut > 0) {
        aTimeOut -= tmpTimeOutHz;
        if (aTimeOut <= 0) {
            LOG(LL_ERROR, "Socket time out is occurred.");
            return RET_ERROR;
        }
        if (aHasPolled) {
            status = FD_WRITABLE;
        } else {
            status = PollSockId(aSockId, aTimeOut, FD_WRITABLE|FD_RWERROR);
            //status = SelectSockId(aSockId, aTimeOut, FD_WRITABLE|FD_RWERROR);
            if (status <= 0 || status & FD_RWERROR) {
                LOG(LL_ERROR, "SelectSockId error status:(%d).", status);
                return RET_ERROR;
            }
        }
        if (status & FD_WRITABLE) {
            if ((n = writev(aSockId, aIovec, aIovCnt)) <= 0) {
                LOG(LL_ERROR, ":while sending packet error:(%s)",
                    strerror(errno));
                return RET_ERROR;
            }
            break;
        }
    }
    DEBUG(LL_ALL, "End");
    return n;
}

// ---------------------------------------------------------------------------
// recv buffer.
// ---------------------------------------------------------------------------
//
int Socket::TcpRecv(int aSockId, char *aBuffer,
                     int aBufSize, int aTimeOut,
                     bool aHasPolled, int aMaxSize) {
    DEBUG(LL_ALL, "Begin");
    if (aBuffer == NULL || aBufSize < 0) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.", aBufSize);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "sockid:(%d), recvsize:(%d), timeout:(%d)",
        aSockId, aBufSize, aTimeOut);
    int n = 0, recvSize = 0, status = FD_READABLE;
    bool isBody = false;
    // aTimeOut *= TIMER_S_TO_MS;
    // using tick hz is instead of time(NULL) when read.
    unsigned int tmpTimeOutHz = aTimeOut / TIMER_TICKS_HZ;
    while (aBufSize > 0) {
        aTimeOut -= tmpTimeOutHz;
        if (aTimeOut <= 0) {
            LOG(LL_ERROR, "Socket time out is occurred.");
            return RET_ERROR;
        }
        if ((status = aHasPolled ? FD_READABLE :
            PollSockId(aSockId, aTimeOut, FD_READABLE)) <= 0) {
            //SelectSockId(aSockId, aTimeOut, FD_READABLE)) <= 0) {
            LOG(LL_ERROR, "SelectSockId error status:(%d).", status);
            return RET_ERROR;
        }
        if (status & FD_READABLE) {
            DEBUG(LL_INFO, "select read set is ok.");
            if ((n = read(aSockId, aBuffer+recvSize, aBufSize)) <= 0) {
                LOG(LL_ERROR, "while read packet error:(%s)", strerror(errno));
                return RET_ERROR;
            }
            aTimeOut += tmpTimeOutHz;
            aBufSize -= n;
            recvSize += n;
            if (!isBody && recvSize >= PKG_HEADER_LEN) {
                isBody = true;
                if (memcmp(aBuffer, PKG_TCP_HEAD_FLAG, PKG_HEADFLAG_LEN) != 0) {
                    LOG(LL_ERROR, "bad tcp cmd header:(%s).", aBuffer);
                    return RET_ERROR;
                }
                unsigned int len = 0;
                memcpy(&len, aBuffer+PKG_HEADFLAG_LEN, sizeof(unsigned int));
                len = ntohl(len);
                if (len > aMaxSize) {
                    LOG(LL_ERROR, "to big tcp pkg len:(%lu) failed.", len);
                    return RET_ERROR;
                }
                aBufSize = static_cast<int>(len);
                recvSize = 0;
                aHasPolled = true;
                continue;
            }
        }
        aHasPolled = false;
    }
    aBuffer[recvSize] = 0x00;
    LOG(LL_VARS, "recv str:");
    LOG_HEX_ASC(LL_VARS, aBuffer, recvSize);
    DEBUG(LL_ALL, "End");
    return recvSize;
}

// ---------------------------------------------------------------------------
// recv buffer once.
// ---------------------------------------------------------------------------
//
int Socket::TcpRecvOnce(int aSockId, char *aBuffer, int aBufSize) {
    DEBUG(LL_ALL, "Begin");
    if (aBuffer == NULL || aBufSize < 0) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.",
            aBufSize);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "sockid:(%d), recvsize:(%d)",
        aSockId, aBufSize);
    int ret = RET_ERROR;
    if((ret = read(aSockId, aBuffer, aBufSize)) <= 0) {
        LOG(LL_ERROR, "sockid:(%d),read error:(%s)", aSockId, strerror(errno));
        return RET_ERROR;
    }
    aBuffer[ret] = 0x00;
    LOG_HEX_ASC(LL_VARS, aBuffer, ret);
    DEBUG(LL_ALL, "End");
    return ret;
}

// ---------------------------------------------------------------------------
// recv buffer as max size.
// ---------------------------------------------------------------------------
//
int Socket::TcpRecvAll(int aSockId, char *aBuffer, int aBufSize, int aTimeOut) {
    DEBUG(LL_ALL, "Begin");
    if (aBuffer == NULL || aBufSize < 0) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.", aBufSize);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "sockid:(%d), recvsize:(%d), timeout:(%d)",
        aSockId, aBufSize, aTimeOut);
    int n = 0, recvSize = 0;
    bool isBody = false;
    // aTimeOut *= TIMER_S_TO_MS;
    // using tick hz is instead of time(NULL) when read.
    unsigned int tmpTimeOutHz = aTimeOut / TIMER_TICKS_HZ;
    while (aBufSize > 0) {
        aTimeOut -= tmpTimeOutHz;
        if (aTimeOut <= 0) {
            LOG(LL_ERROR, "Socket time out is occurred.");
            return RET_ERROR;
        }
        int status = PollSockId(aSockId, aTimeOut, FD_READABLE|FD_RWERROR);
        //int status = SelectSockId(aSockId, aTimeOut, FD_READABLE|FD_RWERROR);
        if (status <= 0 || status & FD_RWERROR) {
            if (recvSize > 0) {
                break;
            }
            LOG(LL_ERROR, "SelectSockId error status:(%d).", status);
            return RET_ERROR;
        }
        if (status & FD_READABLE) {
            DEBUG(LL_INFO, "select read set is ok.");
            n = read(aSockId, aBuffer+recvSize, aBufSize);
            if (n <= 0) {
                LOG(LL_ERROR, "while read packet error:(%s)", strerror(errno));
                break;
            }
            aTimeOut += tmpTimeOutHz;
            aBufSize -= n;
            recvSize += n;
            aBuffer[recvSize] = 0x00;
        }
    }
    aBuffer[recvSize] = 0x00;
    LOG_HEX_ASC(LL_VARS, aBuffer, recvSize);
    DEBUG(LL_ALL, "End");
    return recvSize;
}

// ---------------------------------------------------------------------------
// recv buffer once.
// ---------------------------------------------------------------------------
//
int Socket::TcpRecvHttpHeader(int aSockId, char *aBuffer,
                              int aBufSize, int aTimeOut, bool aHasPolled) {
    DEBUG(LL_ALL, "Begin");
    if (aBuffer == NULL || aBufSize < 0) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.", aBufSize);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "sockid:(%d), recvsize:(%d), timeout:(%d)",
        aSockId, aBufSize, aTimeOut);
    int n = 0, recvSize = 0, status = FD_READABLE;
    bool isBody = false;
    // aTimeOut *= TIMER_S_TO_MS;
    // using tick hz is instead of time(NULL) when read.
    unsigned int tmpTimeOutHz = aTimeOut / TIMER_TICKS_HZ;
    while (aBufSize > 0) {
        aTimeOut -= tmpTimeOutHz;
        if (aTimeOut <= 0) {
            LOG(LL_ERROR, "Socket time out is occurred.");
            return RET_ERROR;
        }
        if ((status = aHasPolled ? FD_READABLE :
            PollSockId(aSockId, aTimeOut, FD_READABLE)) <= 0) {
                //SelectSockId(aSockId, aTimeOut, FD_READABLE)) <= 0) {
                LOG(LL_ERROR, "SelectSockId error status:(%d).", status);
                return RET_ERROR;
        }
        //status = SelectSockId(aSockId, aTimeOut, FD_READABLE|FD_RWERROR);
        if (status & FD_READABLE) {
            DEBUG(LL_INFO, "select read set is ok.");
            if ((n = read(aSockId, aBuffer+recvSize, aBufSize)) <= 0) {
                LOG(LL_ERROR, "while read packet error:(%s)", strerror(errno));
                return RET_ERROR;
            }
            aTimeOut += tmpTimeOutHz;
            aBufSize -= n;
            recvSize += n;
            aBuffer[recvSize] = 0x00;
            if (strstr(aBuffer, "\r\n\r\n") != NULL) {
                break;
            }
        }
        aHasPolled = false;
    }
    aBuffer[recvSize] = 0x00;
    LOG_HEX_ASC(LL_VARS, aBuffer, recvSize);
    DEBUG(LL_ALL, "End");
    return recvSize;
}

int Socket::RecvHttpPkg(int aSockId, char *aBuffer,
                        int aBufSize, int aTimeOut, bool aHasPolled) {
    DEBUG(LL_ALL, "Begin");
    if (aBuffer == NULL || aBufSize < 0) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.", aBufSize);
        return RET_ERROR;
    }
    int recvLen = TcpRecvHttpHeader(aSockId, aBuffer, MAX_SIZE_2K, aTimeOut, aHasPolled);
    if (recvLen == RET_ERROR) {
        LOG(LL_ERROR, "sockid:(%d),recv data error:(%s).",
            aSockId, strerror(errno));
        return RET_ERROR;
    }
    char *ptr = strstr(aBuffer, "\r\n\r\n")+4;
    if (ptr == NULL) {
        LOG(LL_ERROR, "bad HTTP package.");
        return RET_ERROR;
    }
    int hasRecvLen = recvLen - (ptr - aBuffer);
    ptr = strstr(aBuffer, "Content-Length");
    int needRecvLen = 0;
    if (ptr != NULL) {
        ptr += 16;
        char* pEnd = strstr(ptr, "\r\n");
        if (pEnd == NULL) {
            return RET_ERROR;
        }
        *pEnd = 0x00;
        needRecvLen = atoi(ptr);
        *pEnd = '\r';
    }

    needRecvLen -= hasRecvLen;
    DEBUG(LL_VARS, "has recv len:(%d), need recv len:(%d)",
        hasRecvLen, needRecvLen);
    if (needRecvLen + recvLen > aBufSize) {
        LOG(LL_ERROR, "recv too large http body (%d).", needRecvLen);
        return RET_ERROR;
    }
    if (needRecvLen > 0) {
        // continue ro receiving http body.
        if (needRecvLen != TcpRecvByLen(aSockId, aBuffer+recvLen,
            needRecvLen, 0, aTimeOut)) {
            LOG(LL_ERROR, "recv http body error:(%s).", strerror(errno));
            return RET_ERROR;
        }
    }
    DEBUG(LL_ALL, "End");
    return recvLen+needRecvLen;
}

// ---------------------------------------------------------------------------
// recv buffer by line,end of '\n'. aBufSize is max length of aBuffer.
// ---------------------------------------------------------------------------
//
int Socket::TcpRecvLine(int aSockId, char *aBuffer,
                         int aBufSize, int aTimeOut) {
    DEBUG(LL_ALL, "Begin");
    if (aBuffer == NULL || aBufSize < 0) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.", aBufSize);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "sockid:(%d), recvsize:(%d), timeout:(%d)",
        aSockId, aBufSize, aTimeOut);
    int n = 0, recvSize = 0;
    bool isBody = false;
    // aTimeOut *= TIMER_S_TO_MS;
    // using tick hz is instead of time(NULL) when read.
    unsigned int tmpTimeOutHz = aTimeOut / TIMER_TICKS_HZ;
    while (aBufSize > 0) {
        aTimeOut -= tmpTimeOutHz;
        if (aTimeOut <= 0) {
            LOG(LL_ERROR, "Socket time out is occurred.");
            return RET_ERROR;
        }
        int status = PollSockId(aSockId, aTimeOut, FD_READABLE|FD_RWERROR);
        //int status = SelectSockId(aSockId, aTimeOut, FD_READABLE|FD_RWERROR);
        if (status <= 0 || status & FD_RWERROR) {
            LOG(LL_ERROR, "SelectSockId error status:(%d).", status);
            return RET_ERROR;
        }
        if (status & FD_READABLE) {
            DEBUG(LL_INFO, "select read set is ok.");
            aTimeOut += tmpTimeOutHz;
            do {
                if ((n = read(aSockId, aBuffer+recvSize, 1)) <= 0) {
                    LOG(LL_ERROR, "while read packet error:(%s)",
                        strerror(errno));
                    return RET_ERROR;
                }
                if (*(aBuffer+recvSize) == '\n') {
                    *(aBuffer+recvSize) = 0x00;
                    if (recvSize > 2 && *(aBuffer+recvSize-1) == '\r') {
                        *(aBuffer+recvSize-1) = 0x00;
                        recvSize--;
                    }
                    break;
                }
                aBufSize -= n;
                recvSize += n;
            } while (aBufSize > 0);
            break;
        }
    }
    aBuffer[recvSize] = 0x00;
    LOG_HEX_ASC(LL_VARS, aBuffer, recvSize);
    DEBUG(LL_ALL, "End");
    return recvSize;
}

// ---------------------------------------------------------------------------
// recv buffer by XXXXyyyyy. XXXX is hex len and yyyy is buffer.
// ---------------------------------------------------------------------------
//
int Socket::TcpRecvByLen(int aSockId, char *aBuffer, int aBufSize,
                         int aFlag, int aTimeOut, bool aHasPolled) {
    DEBUG(LL_ALL, "Begin");
    if (aBuffer == NULL || aBufSize < 0) {
        LOG(LL_ERROR, "buffer is null or size:(%d) less than zero.",
            aBufSize);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "sockid:(%d), recvsize:(%d), timeout:(%d)",
        aSockId, aBufSize, aTimeOut);
    int n = 0, recvSize = 0, status = FD_READABLE;
    bool isBody = false;
    // aTimeOut *= TIMER_S_TO_MS;
    // using tick hz is instead of time(NULL) when read.
    unsigned int tmpTimeOutHz = aTimeOut / TIMER_TICKS_HZ;
    while (aBufSize > 0) {
        aTimeOut -= tmpTimeOutHz;
        if (aTimeOut <= 0) {
            LOG(LL_ERROR, "Socket time out is occurred.");
            return RET_ERROR;
        }
        if ((status = aHasPolled ? FD_READABLE :
            PollSockId(aSockId, aTimeOut, FD_READABLE)) <= 0) {
            //SelectSockId(aSockId, aTimeOut, FD_READABLE)) <= 0) {
            LOG(LL_ERROR, "SelectSockId error status:(%d).", status);
            return RET_ERROR;
        }
        if (status & FD_READABLE) {
            DEBUG(LL_INFO, "select read set is ok.");
            if ((n = read(aSockId, aBuffer+recvSize, aBufSize)) <= 0) {
                LOG(LL_ERROR, "while read packet error:(%s)", strerror(errno));
                return RET_ERROR;
            }
            aTimeOut += tmpTimeOutHz;
            aBufSize -= n;
            recvSize += n;
            if (aFlag != 0 && !isBody && recvSize >= PKG_CMDLEN_LEN) {
                isBody = true;
                aBuffer[recvSize] = 0x00;
                aBufSize = (aFlag == 16) ? LenXHex2Int(aBuffer) : atoi(aBuffer);
                if (aBufSize <= 0) {
                    LOG(LL_ERROR, "fetch tcp pkg len:(%s) failed.", aBuffer);
                    return RET_ERROR;
                }
                recvSize = 0;
            }
        }
        aHasPolled = false;
    }
    aBuffer[recvSize] = 0x00;
    LOG(LL_VARS, "recv str:");
    LOG_HEX_ASC(LL_VARS, aBuffer, recvSize);
    DEBUG(LL_ALL, "End");
    return recvSize;
}

// ---------------------------------------------------------------------------
// Create Package header.
// 'YF'dddd  'YF' is identity and dddd is length encoding by Integer ntohl.
// ---------------------------------------------------------------------------
//
int Socket::CreatePkgHeader(int aLen, char *aOutBuf, const string& aAppStr) {
    unsigned int len = static_cast<unsigned int>(aLen+aAppStr.length());
    len = htonl(len);
    memcpy(aOutBuf, PKG_TCP_HEAD_FLAG, PKG_HEADFLAG_LEN);
    memcpy(aOutBuf+PKG_HEADFLAG_LEN, &len, sizeof(unsigned int));
    if (!aAppStr.empty()) {
        memcpy(aOutBuf+PKG_HEADER_LEN, aAppStr.data(), aAppStr.length());
    }

    return PKG_HEADFLAG_LEN+sizeof(unsigned int)+aAppStr.length();
}
// ---------------------------------------------------------------------------
// hex string to int.
// ---------------------------------------------------------------------------
//
unsigned int Socket::LenXHex2Int(const char* aStr, int len) {
    unsigned int i, res = 0;

    for(i = 0; i < len; i++) {
        res *= 16;
        if ((aStr[i] >= '0') && (aStr[i] <= '9')) {
            res += aStr[i] - '0';
        } else if ((aStr[i] >= 'a') && (aStr[i] <= 'f')) {
            res += aStr[i] - 'a' + 10;
        } else if ((aStr[i] >= 'A') && (aStr[i] <= 'F')) {
            res += aStr[i] - 'A' + 10;
        } else {
            return 0;
        }
    }
    return res;
}

// ---------------------------------------------------------------------------
// hex to int
// ---------------------------------------------------------------------------
//
int Socket::Hex2Int(const char hex_digit) {
    if (hex_digit >= '0' && hex_digit <= '9') return hex_digit-'0';
    if (hex_digit >= 'a' && hex_digit <= 'f') return hex_digit-'a'+10;
    if (hex_digit >= 'A' && hex_digit <= 'F') return hex_digit-'A'+10;
    return -1;
}


// ---------------------------------------------------------------------------
// getting local ip address and ignore local loopback ip.
// ---------------------------------------------------------------------------
//
int Socket::GetLocalIP(char* aOutIP) {
    int i=0;
    int sockfd;
    struct ifconf ifconf;
    char buf[512];
    struct ifreq *ifreq;
    char* ip;
    // ifconf initialization.
    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < RET_OK) {
        return RET_ERROR;
    }
    // get all interface information.
    ioctl(sockfd, SIOCGIFCONF, &ifconf);
    close(sockfd);
    // get ip address one by one.
    ifreq = (struct ifreq*)buf;
    for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i > 0; i--) {
        ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
        // local loopback ip is ignoring, finding next one.
        if(strcmp(ip,"127.0.0.1")==0) {
            ifreq++;
            continue;
        }
        strcpy(aOutIP, ip);
        return RET_OK;
    }

    return RET_ERROR;
}

// ---------------------------------------------------------------------------
// get tcp information structure.
// ---------------------------------------------------------------------------
//
bool Socket::GetTcpInfo(int fd, struct tcp_info* tcpi) const
{
  socklen_t len = sizeof(*tcpi);
  memset(tcpi, 0, len);
  return ::getsockopt(fd, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

// ---------------------------------------------------------------------------
// get tcp information string.
// ---------------------------------------------------------------------------
//
bool Socket::GetTcpInfoString(int fd, char* buf, int len) const
{
    struct tcp_info tcpi;
    bool ok = GetTcpInfo(fd, &tcpi);
    if (ok) {
        snprintf(buf, len, "unrecovered=%u "
            "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
            "lost=%u retrans=%u rtt=%u rttvar=%u "
            "sshthresh=%u cwnd=%u total_retrans=%u",
            tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
            tcpi.tcpi_rto,          // Retransmit timeout in usec
            tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
            tcpi.tcpi_snd_mss,
            tcpi.tcpi_rcv_mss,
            tcpi.tcpi_lost,         // Lost packets
            tcpi.tcpi_retrans,      // Retransmitted packets out
            tcpi.tcpi_rtt,          // Smoothed round trip time in usec
            tcpi.tcpi_rttvar,       // Medium deviation
            tcpi.tcpi_snd_ssthresh,
            tcpi.tcpi_snd_cwnd,
            tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
    }
    return ok;
}

// ---------------------------------------------------------------------------
// whether ip is local or not.
// ---------------------------------------------------------------------------
//
bool Socket::HostNameIsLocal(const string& aName) {
    return aName == "localhost" ||
        aName ==  "::1" ||
        aName.compare(0, 4, "127.");
}
// end of local file.
