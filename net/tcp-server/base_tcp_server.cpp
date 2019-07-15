#include "BaseTcpServer.h"
#include "logger.h"

namespace common {

void callback_on_accept(evutil_socket_t fd, short event, void* arg) {
    reinterpret_cast<BaseTcpServer*>(arg)->on_accept(fd, event, arg);
}

void callback_on_read(struct bufferevent* bev, void* arg) {
    reinterpret_cast<BaseTcpServer*>(arg)->on_read(bev, arg);
}

void callback_on_write(struct bufferevent* bev, void* arg) {
    reinterpret_cast<BaseTcpServer*>(arg)->on_write(bev, arg);
}

void callback_on_other_event(struct bufferevent* bev, short event, void* arg) {
    reinterpret_cast<BaseTcpServer*>(arg)->on_other_event(bev, event, arg);
}

int BaseTcpServer::run() {
    
    // 初始化socket并绑定到libevent上
    evutil_socket_t fd;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (fd <= 0) {
        LERROR("msg=[create socket fail] detail=[fd <= 0]");
        return ERR_CREATE_SOCKET;
    }

    // 开启reuseaddr
    evutil_make_listen_socket_reuseable(fd);
    
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = _ipv4addr;
    sin.sin_port = htons(_port);
    
    // 绑定socket
    if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        LERROR("msg=[bind socket fail]");
        return ERR_BIND_SOCKET;
    }
    
    // 监听socket

    if (listen(fd, SOMAXCONN) < 0) {
        LERROR("msg=[listen socket fail]");
        return ERR_LISTEN_SOCKET;
    }
    
    evutil_make_socket_nonblocking(fd);
    
    // 创建libevent对象
    struct event_base* base = event_base_new();
    if (base == NULL) {
        LERROR("msg=[create event base fail] detail=[null event base]");
        return ERR_EVENT_NULLPTR;
    }
    
    _base = base;
    
    // 创建libevent监听
    struct event* listen_event;

    listen_event = event_new(_base, fd, EV_READ|EV_PERSIST, &::common::callback_on_accept, (void*)this);
    event_add(listen_event, NULL);
    event_base_dispatch(_base);
    event_base_free(_base);

    return ERR_NONE;
}

void BaseTcpServer::on_accept(evutil_socket_t fd, short event, void *arg) {
    //printf("accepted!\n");
    struct event_base *base = _base;
    evutil_socket_t fd;
    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    
    fd = accept(fd, (struct sockaddr *)&sin, &slen);
    if (fd < 0) {
        LERROR("msg=[accept conn fail] detail=[accept fd < 0]");
        return;
    }
    
    evutil_make_socket_nonblocking(fd);
    
    //printf("accepted: fd = %u\n", fd);
    
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, &::common::callback_on_read, &::common::callback_on_write, &::common::callback_on_other_event, arg);
    
    struct timeval tv = {60, 0}; // 60秒超时 没有数据写入将断开连接
    bufferevent_set_timeouts(bev, &tv, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST);
}

void BaseTcpServer::on_read(struct bufferevent* bev, void* arg) {
    // 去子类中实现
}

void BaseTcpServer::on_write(struct bufferevent* bev, void* arg) {
    // 去子类中实现
}

void BaseTcpServer::on_other_event(struct bufferevent* bev, short event, void* arg) {
    // 去子类中实现
}

}