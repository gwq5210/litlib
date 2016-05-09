/*************************************************************************
	> File Name: server_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月07日 星期六 23时08分44秒
 ************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>

#include "server_helper.h"
#include "common_tool.h"
#include "error.h"
#include "epoll_helper.h"

namespace util {


int server_helper::init(const char *ip, int port)
{
    if (*ip == '\0' || port < 0) {

        set_errmsg(errmsg, "ip is NULL or port less then zero!");
        return ERR_PARAM;
    }

    errmsg = sdsnewlen(1024);
    if (errmsg == NULL) {
        return ERR_LIBCALL;
    }
    server_ip = sdsnew(ip);
    if (server_ip == NULL) {
        set_errmsg(errmsg, "sdsnew error! server_ip is NULL!");
        return ERR_LIBCALL;
    }
    server_port = port;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        set_errmsg(errmsg, "socket error! errno[%d], %s", errno, strerror(errno));
        return ERR_SYSCALL;
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) < 0) {
        set_errmsg(errmsg, "inet_pton ip[%s] error! errno[%d], %s", server_ip, errno, strerror(errno));
        return ERR_SYSCALL;
    }

    int ret = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        set_errmsg(errmsg, "socket %s:%d error! errno[%d], %s", server_ip, port, errno, strerror(errno));
        return ERR_SYSCALL;
    }

    return 0;
}

int server_helper::accept_client(sds &client_ip, int &client_port)
{
    if (client_ip == NULL) {
        set_errmsg(errmsg, "error param! client ip is NULL!");
        return ERR_PARAM;
    }

    struct sockaddr_in addr;
    int addr_size = sizeof(addr);
    int socket_fd = accept(server_fd, (struct sockaddr *)&addr, (socklen_t *)&addr_size);
    if (socket_fd < 0) {
        set_errmsg(errmsg, "accept error! errno[%d], %s", errno, strerror(errno));
        return ERR_SYSCALL;
    }

    client_port = ntohs(addr.sin_port);
    char *ret_ip = inet_ntoa(addr.sin_addr);
    if (ret_ip != NULL) {
        sdscpy(client_ip, ret_ip);
    }

    return socket_fd;
}

int server_helper::do_write(int client_fd, const char *write_buf, int buflen)
{
    if (client_fd < 0) {
        set_errmsg(errmsg, "param error! client fd less then zero!");
        return ERR_PARAM;
    }
    if (write_buf == NULL || buflen <= 0) {
        set_errmsg(errmsg, "param error! write buf is NULL or buflen less equal zero!");
        return ERR_PARAM;
    }

    return socket_write(client_fd, write_buf, buflen, errmsg);
}

int server_helper::do_read(int client_fd, sds &read_buf, int timeout_ms/* = -1*/, int len/* = -1*/)
{
    if (client_fd < 0) {
        set_errmsg(errmsg, "param error! client fd less then zero!");
        return ERR_NOINIT;
    }

    if (read_buf == NULL) {
        set_errmsg(errmsg, "param error! read_buf is NULL");
        return ERR_PARAM;
    }

    return socket_read(client_fd, read_buf, errmsg, timeout_ms, len);
}

#ifdef SERVER_HELPER_TEST

static int handle_accept(server_helper &svr, int timeout_ms)
{
    sds client_ip = sdsnewlen(1024);
    int client_port = -1;
    int client_fd = svr.accept_client(client_ip, client_port);
    if (client_fd < 0) {
        printf("accept client error! %s\n", svr.get_errmsg());
        return -1;
    } else {
        printf("accept client %s:%d!\n", client_ip, client_port);
    }

    const char *write_buf = "hello world!";
    int buflen = strlen(write_buf);
    if (svr.do_write(client_fd, write_buf, buflen) < 0) {
        printf("write client fd[%d], error! %s\n", client_fd, svr.get_errmsg());
        return -1;
    } else {
        printf("write to client %s:%d buf: %s\n", client_ip, client_port, write_buf);
    }

    sds read_buf = sdsnewlen(1024);
    svr.do_read(client_fd, read_buf, timeout_ms, 3);
    if (svr.do_read(client_fd, read_buf, timeout_ms, 2) < 0) {
        printf("read client fd[%d], error! %s\n", client_fd, svr.get_errmsg());
        return -1;
    } else {
        printf("read from %s:%d buf: %s\n", client_ip, client_port, read_buf);
    }

    if (svr.do_write(client_fd, write_buf, buflen) < 0) {
        printf("write client fd[%d], error! %s\n", client_fd, svr.get_errmsg());
        return -1;
    } else {
        printf("write to client %s:%d buf: %s\n", client_ip, client_port, write_buf);
    }

    if (svr.do_read(client_fd, read_buf, timeout_ms) < 0) {
        printf("read client fd[%d], error! %s\n", client_fd, svr.get_errmsg());
        return -1;
    } else {
        printf("read from %s:%d buf: %s\n", client_ip, client_port, read_buf);
    }

    sleep(1);
    svr.close_client(client_fd);

    sdsfree(client_ip);
    sdsfree(read_buf);

    return 0;
}

int server_helper_test()
{
    server_helper svr;
    const char *server_ip = "127.0.0.1";
    int server_port = 8000;
    int timeout_ms = -1;
    int ret = svr.init(server_ip, server_port);
    if (ret < 0) {
        printf("server init error! %s\n", svr.get_errmsg());
        return -1;
    }

    ret = svr.start_listen();
    if (ret < 0) {
        printf("server listen error! %s\n", svr.get_errmsg());
        return -1;
    } else {
        printf("start listen %s:%d\n", server_ip, server_port);
    }

    int server_fd = svr.get_server_fd();
    epoll_helper epoll;
    epoll.create(64);
    epoll.add(server_fd, EPOLLIN);

    int times = 6;
    while (times--) {
        printf("times = %d\n", times + 1);
        int ret = epoll.wait(-1);
        if (ret < 0) {
            printf("wait error! %s\n", epoll.get_errmsg());
            break;
        } else if (ret == 0) {
            printf("no fd reday!\n");
            sleep(1);
            continue;
        }

        printf("%d fd ready!\n", ret);
        int evt;
        long long usr_data;
        while (epoll.get_events(evt, usr_data) == 0) {
            if (EPOLLIN & evt) {
                if (server_fd == usr_data) {
                    printf("fd[%d] ready!\n", server_fd);
                    handle_accept(svr, timeout_ms);
                }
            } else {
                printf("event error!\n");
                continue;
            }
        }
    }

    return 0;
}

#endif      // SERVER_HELPER_TEST

}
