/*
 * tcp_epoll.cpp
 *
 *  Created on: Nov 10, 2014
 *      Author: liao
 */
#include <cstdlib>
#include <climits>
#include <cstdio>
#include <cerrno>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>

#include "log.h"
#include "epoll_socket.h"

int EpollSocket::setNonblocking(int fd) {
    int flags;

    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
};

int EpollSocket::listen_on(int port, int backlog) {
    int sockfd; /* listen on sock_fd, new connection on new_fd */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in my_addr; /* my address information */
    memset (&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET; /* host byte order */
    my_addr.sin_port = htons(port); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        exit(1);
    }

    log_warn(ERRNO_OK, "start Server Socket on port :" << port);
    return sockfd;
}

int EpollSocket::accept_socket(int sockfd, std::string &client_ip) {
    int new_fd;
    struct sockaddr_in their_addr; /* connector's address information */
    socklen_t sin_size = sizeof(struct sockaddr_in);

    if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
        perror("accept");
        return -1;
    }

    client_ip = inet_ntoa(their_addr.sin_addr);
    log_debug("server: got connection from " << client_ip);
    return new_fd;
}

int EpollSocket::handle_accept_event(int &epollfd, epoll_event &event, EpollSocketWatcher &socket_handler) {
    int sockfd = event.data.fd;

    std::string client_ip;
    int conn_sock = accept_socket(sockfd, client_ip);
    if (conn_sock == -1) {
        return -1;
    }
    setNonblocking(conn_sock);
    log_debug("get accept socket which listen fd:" << sockfd << ", conn_sock_fd:" << conn_sock);

    EpollContext *epoll_context = new EpollContext();
    epoll_context->fd = conn_sock;
    epoll_context->client_ip = client_ip;

    socket_handler.on_accept(*epoll_context);

    struct epoll_event conn_sock_ev;
    conn_sock_ev.events = EPOLLIN | EPOLLET;
    conn_sock_ev.data.ptr = epoll_context;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &conn_sock_ev) == -1) {
        perror("epoll_ctl: conn_sock");
        return -1;
    }

    return 0;
}

int EpollSocket::handle_readable_event(int &epollfd, epoll_event &event, EpollSocketWatcher &socket_handler) {
    EpollContext *epoll_context = (EpollContext *) event.data.ptr;
    int fd = epoll_context->fd;

    int buffer_size = SS_READ_BUFFER_SIZE;
    char read_buffer[buffer_size];
    memset(read_buffer, 0, buffer_size);

    int read_size = recv(fd, read_buffer, buffer_size, 0);

    int handle_ret = 0;
    if(read_size > 0) {
        log_debug("read success which read size:" << read_size);
        handle_ret = socket_handler.on_readable(*epoll_context, read_buffer, buffer_size, read_size);
    }

    if(read_size <= 0 /* connect close or io error*/ || handle_ret < 0) {
        close_and_release(epollfd, event, socket_handler);
        return 0;
    }

    if (handle_ret == READ_CONTINUE) {
        event.events = EPOLLIN | EPOLLET;
    } else {
        event.events = EPOLLOUT | EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    return 0;
}

int EpollSocket::handle_writeable_event(int &epollfd, epoll_event &event, EpollSocketWatcher &socket_handler) {
    EpollContext *epoll_context = (EpollContext *) event.data.ptr;
    int fd = epoll_context->fd;
    log_debug("start write data");

    int ret = socket_handler.on_writeable(*epoll_context);
    if(ret == WRITE_CONN_CLOSE) {
        close_and_release(epollfd, event, socket_handler);
        return 0;
    }

    if (ret == WRITE_CONN_CONTINUE) {
        event.events = EPOLLOUT | EPOLLET;
    } else {
        event.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    return 0;
}

int EpollSocket::start_epoll(int port, EpollSocketWatcher &socket_handler, int backlog, int max_events) {
    int sockfd = this->listen_on(port, backlog);

    int epollfd = epoll_create(1024);
    if (epollfd == -1) {
        perror("epoll_create");
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        return -1;
    }

    epoll_event *events = new epoll_event[max_events];

    while(1) {
        int fds_num = epoll_wait(epollfd, events, max_events, -1);
        if(fds_num == -1) {
            perror("epoll_pwait");
            sleep(1);
            continue;
        }

        for (int i = 0; i < fds_num; i++) {
            if(events[i].data.fd == sockfd) {
                // accept connection
                this->handle_accept_event(epollfd, events[i], socket_handler);
            } else if(events[i].events & EPOLLIN ){
                // readable
                this->handle_readable_event(epollfd, events[i], socket_handler);
            } else if(events[i].events & EPOLLOUT) {
                // writeable
                this->handle_writeable_event(epollfd, events[i], socket_handler);
            } else {
                log_fatal(ERRNO_HTTP_ERR, "unkonw events:" << events[i].events);
            }
        }
    }

    if (events != NULL) {
        delete[] events;
        events = NULL;
    }
    return 0;
}

int EpollSocket::close_and_release(int &epollfd, epoll_event &epoll_event, EpollSocketWatcher &socket_handler) {
    if(epoll_event.data.ptr == NULL) {
        return 0;
    }
    log_debug("connect close");

    EpollContext *hc = (EpollContext *) epoll_event.data.ptr;

    socket_handler.on_close(*hc);

    int fd = hc->fd;
    epoll_event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &epoll_event);

    delete (EpollContext *) epoll_event.data.ptr;
    epoll_event.data.ptr = NULL;

    int ret = close(fd);
    log_debug("connect close complete which fd:" << fd << ",ret:" << ret);
    return ret;
}
