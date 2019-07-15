#ifndef __BASE_TCP_SERVER_H_
#define __BASE_TCP_SERVER_H_

#include <stdio.h>
#include <event2/event.h>
#include <event2/bufferevent.h>


// wrapper of libevent
// single thread + libevent base(epoll)

namespace common {

#define LERROR printf

    class BaseHandler {
    public:
        BaseHandler() {}
        virtual ~BaseHandler() {}
        /**
         * ret: 
         * -1 err
         * > 0 handle package len
         * == 0 no buffer read.
         */
        virtual int check_complate(const char *buf, int len) = 0;

        /**
        * do callback function
        */
        virtual int do_read(const char *buf, int len) = 0;
    };

    class BaseTcpServer {
    public:
        
        // const
        static const int kListenPort = 1234;
        static const size_t kBuffSize = 4096;
        
        enum {
            ERR_NONE                  = 0,
            ERR_CREATE_SOCKET         = -1,
            ERR_BIND_SOCKET           = -2,
            ERR_LISTEN_SOCKET         = -3,
            ERR_EVENT_NULLPTR         = -4
        };
        
        // constructors
        BaseTcpServer(int ipv4addr = 0, int port = kListenPort) : _ipv4addr(ipv4addr), _port(port) {
            _baseHandler = NULL;
        }

        virtual ~BaseTcpServer() {
            if (NULL != _baseHandler) {
                delete _baseHandler;
            }
        }

        int run();
        void set_listen_port(int port) { _port = port; }
        void set_listen_ipv4addr(int ipv4addr) { _ipv4addr = ipv4addr; }
        void set_base_handler(BaseHandler *handlerPtr) {
            _baseHandler = handlerPtr;
        }

        BaseHandler *getBaseHandler() {

        }
        
        // callback functions
        virtual void on_accept(evutil_socket_t listen_fd, short event, void* arg);
        virtual void on_read(struct bufferevent* bev, void* arg);
        virtual void on_write(struct bufferevent* bev, void* arg);
        virtual void on_other_event(struct bufferevent* bev, short event, void* arg);

    private:
        int _ipv4addr;
        int _port;
        struct event_base* _base;
        BaseHandler *_baseHandler;
    };

    // callback function for help

    void callback_on_accept(evutil_socket_t listen_fd, short event, void* arg);
    void callback_on_read(struct bufferevent* bev, void* arg);

    void callback_on_write(struct bufferevent* bev, void* arg);
    
    void callback_on_other_event(struct bufferevent* bev, short event, void* arg);
}

#endif