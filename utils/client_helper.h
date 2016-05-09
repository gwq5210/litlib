/*************************************************************************
	> File Name: client_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月06日 星期五 14时19分46秒
 ************************************************************************/

#ifndef _CLIENT_HELPER_H
#define _CLIENT_HELPER_H

#include <unistd.h>

#include "error.h"
#include "sds.h"
#include "epoll_helper.h"

namespace util {

class client_helper {
public:
    client_helper(): socket_fd(-1), server_ip(NULL), server_port(-1), errmsg(NULL) { init(); epoll.create(64); }
    ~client_helper() { clear(); }
    int conn(const char *ip, int port, int timeout_ms = -1);
    int reconnect(const char *ip = "", int port = -1, int timeout_ms = -1)
    {
        clear();
        init();
        if (*ip == '\0' || port < 0) {
            return conn(server_ip, server_port, timeout_ms);
        } else {
            return conn(ip, port, timeout_ms);
        }
    }
    int do_read(sds &read_buf, int timeout_ms = -1, int len = -1);
    int do_write(const char *write_buf, int buflen);
    int init()
    {
        socket_fd = -1;
        server_port = -1;
        server_ip = sdsnewlen(64);
        if (server_ip == NULL) {
            return ERR_INIT;
        }
        errmsg = sdsnewlen(1024);
        if (errmsg == NULL) {
            return ERR_INIT;
        }
        return 0;
    }
    int clear()
    {
        if (socket_fd >= 0) {
            close(socket_fd);
            socket_fd = -1;
        }
        server_port = -1;
        sdsfree(server_ip);
        sdsfree(errmsg);
        return 0;
    }
    const sds get_errmsg() { return errmsg; }
private:
    int socket_fd;
    sds server_ip;
    int server_port;
    sds errmsg;
    epoll_helper epoll;
};

#ifdef CLIENT_HELPER_TEST

int client_helper_test();

#endif      // CLIENT_HELPER_TEST

}

#endif      // _CLIENT_HELPER_H
