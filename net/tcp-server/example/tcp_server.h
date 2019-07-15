#include "../base_tcp_server.h"

namespace common { namespace example {
class tcp_server : public common::base_tcp_server {
public:
	tcp_server() {}
    virtual void on_read(struct bufferevent *bev, void *arg);
    virtual void on_write(struct bufferevent *bev, void *arg);
    virtual void on_other_event(struct bufferevent* bev, short event, void* arg);
};
}}