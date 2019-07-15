#include "tcp_server.h"
#include <event2/buffer.h>

namespace common { namespace example {
void tcp_server::on_other_event(struct bufferevent *bev, short event, void *arg) {
    if (event & BEV_EVENT_TIMEOUT) {
        LERROR("libevent timeout");
    }
    else if (event & BEV_EVENT_EOF) {
        LERROR("libevent connection closed");
    }
    else if (event & BEV_EVENT_ERROR) {
        LERROR("libevent other error");
    }
    
    // 关闭连接 释放资源
    bufferevent_free(bev);
}


void tcp_server::on_read(struct bufferevent* bev, void* arg) {
    size_t offset = 0;
    evbuffer* evbuff = bufferevent_get_input(bev);
    size_t len = evbuffer_get_length(evbuff);
    const char* data = (const char*)evbuffer_pullup(evbuff, len);
    int count = 0;

    BaseHandler *handler = get_base_handler();
    if (NULL == handler) {
    	bufferevent_free(bev);
    	return;
    }
    while(true) {
        int ret = handler->check_complate(data+offset, len-offset);
        if(ret == -1) {
            LERROR("check package fail");
            // 关闭连接 释放资源
            bufferevent_free(bev);
            return;
        } else if(ret > 0) {
            // 收到数据，处理数据
            handler->do_read(data+offset, len-offset);

            // 收到一个完整的pb包 从evbuffer中删除相应字节
            int res = evbuffer_drain(evbuff, ret);
            if (res != 0) {
                LERROR("libevent drain fail");
                bufferevent_free(bev);
                return;
            }
            offset += ret;
        } else {
            // 未收完的包 evbuffer已经空了 等待下次on_read事件继续处理
            return;
        }
        count++;
    }
    
    // 不关闭连接 以支持长连接方式写入
}

void tcp_server::on_read(struct bufferevent* bev, void* arg) {
	// todo
}

}}