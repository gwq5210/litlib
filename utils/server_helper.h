/*************************************************************************
	> File Name: server_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月07日 星期六 23时08分51秒
 ************************************************************************/

#ifndef _SERVER_HELPER_H
#define _SERVER_HELPER_H

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "sds.h"
#include "ret_status.h"
#include "common_tool.h"

#define LISTEN_QUEUE_SIZE 64

namespace util {

class server_helper {
public:
    server_helper():server_fd(-1), server_ip(NULL), server_port(-1), errmsg(NULL) {}
    ~server_helper() { clear(); }
    int do_write(int client_fd, const char *write_buf, int buflen);
    int do_read(int client_fd, sds &read_buf, int timeout_ms = -1, int len = -1);
    int close_client(int client_fd)
    {
        if (client_fd >= 0) {
            close(client_fd);
        }
        return 0;
    }
    int init(const char *ip, int port);
    int start_listen()
    {
        if (server_fd < 0) {
            set_errmsg(errmsg, "no init! server fd is less zero!");
            return ERR_NOINIT;
        }
        if (listen(server_fd, LISTEN_QUEUE_SIZE) < 0) {
            set_errmsg(errmsg, "listen %s:%d error! errno[%d], %s!", server_ip, server_port, errno, strerror(errno));
            return ERR_SYSCALL;
        }
        return 0;
    }
    int accept_client(sds &client_ip, int &client_port);
    int get_server_fd() { return server_fd; }
    const sds get_server_ip() { return server_ip; }
    int get_server_port() { return server_port; }
    const sds get_errmsg() { return errmsg; }
    int clear()
    {
        if (server_fd >= 0) {
            close(server_fd);
            server_fd = -1;
        }
        sdsfree(server_ip);
        sdsfree(errmsg);
        server_port = -1;
        return 0;
    }
private:
    int server_fd;
    sds server_ip;
    int server_port;
    sds errmsg;
};

#ifdef SERVER_HELPER_TEST

int server_helper_test();

#endif      // SERVER_HELPER_TEST

}

#endif      // _SERVER_HELPER_H
