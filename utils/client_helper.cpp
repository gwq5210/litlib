/*************************************************************************
	> File Name: client_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月06日 星期五 14时19分32秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "client_helper.h"
#include "common_tool.h"

namespace util {

int client_helper::conn(const char *ip, int port, int timeout_ms/* = -1*/)
{
    if (*ip == '\0' || port < 0) {
        set_errmsg(errmsg, "error param! ip[%s], port[%d]", ip, port);
        return ERR_PARAM;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        set_errmsg(errmsg, "socket error! errno[%d], %s", errno, strerror(errno));
        return ERR_SYSCALL;
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    sdscpy(server_ip, ip);
    server_port = port;
    if (inet_pton(AF_INET, ip, &addr.sin_addr) < 0) {
        set_errmsg(errmsg, "inet_pton ip[%s] error! errno[%d], %s", ip, errno, strerror(errno));
        return ERR_SYSCALL;
    }

    int flag = 0;
    if (timeout_ms >= 0) {
        flag = fcntl(socket_fd, F_GETFL) | O_NONBLOCK;
        if (flag < 0) {
            set_errmsg(errmsg, "fcntl get fd[%d] flag error! errno[%d], %s", socket_fd, errno, strerror(errno));
            return ERR_SYSCALL;
        }
        if (fcntl(socket_fd, F_SETFL, flag) < 0) {
            set_errmsg(errmsg, "fcntl set fd[%d] flag[%d] error! errno[%d], %s", socket_fd, flag, errno, strerror(errno));
            return ERR_SYSCALL;
        }
    }

    int ret = connect(socket_fd, (sockaddr *)&addr, sizeof(addr));
    if (ret < 0 && errno != EINPROGRESS) {
        set_errmsg(errmsg, "connect %s:%d error! errno[%d], %s", ip, port, errno, strerror(errno));
        return ERR_SYSCALL;
    }

    if (timeout_ms >= 0) {
        epoll.add(socket_fd, EPOLLIN);
        ret = epoll.wait(timeout_ms);
        if (ret < 0) {
            set_errmsg(errmsg, "epoll wait error! %s", epoll.get_errmsg());
            return ERR_LIBCALL;
        } else if (ret == 0) {
            set_errmsg(errmsg, "connect %s:%d time out[%dms]! %s", ip, port, timeout_ms, epoll.get_errmsg());
            return ERR_TIMEOUT;
        }

        int socket_ret = 0;
        int socket_ret_len = sizeof(int);
        if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &socket_ret, (socklen_t *)&socket_ret_len) < 0) {
            set_errmsg(errmsg, "getsockopt fd[%d] error! errno[%d], %s", socket_fd, errno, strerror(errno));
            return ERR_SYSCALL;
        }

        if (socket_ret != 0) {
            set_errmsg(errmsg, "connect %s:%d error! errno[%d], %s", ip, port, errno, strerror(errno));
            return ERR_SYSCALL;
        }

        flag &= ~O_NONBLOCK;
        if (fcntl(socket_fd, F_SETFL, flag) < 0) {
            set_errmsg(errmsg, "fcntl set fd[%d] flag[%d] error! errno[%d], %s", socket_fd, flag, errno, strerror(errno));
            return ERR_SYSCALL;
        }
    }

    return 0;
}

int client_helper::do_read(sds &read_buf, int timeout_ms/* = -1*/, int len/* = -1*/)
{
    if (read_buf == NULL) {
        set_errmsg(errmsg, "param error! read_buf is NULL");
        return ERR_PARAM;
    }

    if (socket_fd < 0) {
        set_errmsg(errmsg, "no connect! socket_fd[%d]", socket_fd);
        return ERR_NOINIT;
    }
    return socket_read(socket_fd, read_buf, errmsg, timeout_ms, len);
}

int client_helper::do_write(const char *write_buf, int buflen)
{
    if (write_buf == NULL || buflen <= 0) {
        set_errmsg(errmsg, "param error! write_buf is NULL or buflen less then zero!");
        return ERR_PARAM;
    }

    if (socket_fd < 0) {
        set_errmsg(errmsg, "no connect! socket_fd[%d]", socket_fd);
        return ERR_NOINIT;
    }

    return socket_write(socket_fd, write_buf, buflen, errmsg);
}

#ifdef CLIENT_HELPER_TEST

int client_helper_test()
{
    client_helper clt;
    const char *server_ip = "127.0.0.1";
    int server_port = 8000;
    int timeout_ms = 10000;
    if (clt.conn(server_ip, server_port, timeout_ms) < 0) {
        printf("connect timeout! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("connect to server %s:%d success!\n", server_ip, server_port);
    }

    sds read_buf = sdsnewlen(1024);
    if (clt.do_read(read_buf, timeout_ms) < 0) {
        printf("read error! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("read from server %s:%d buf: %s\n", server_ip, server_port, read_buf);
    }

    const char *msg = "nihao";
    int msglen = strlen(msg);
    if (clt.do_write(msg, msglen) < 0) {
        printf("write error! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("write to server %s:%d buf[%d]: %s\n", server_ip, server_port, msglen, msg);
    }

    if (clt.do_read(read_buf, timeout_ms) < 0) {
        printf("read error! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("read from server %s:%d buf: %s\n", server_ip, server_port, read_buf);
    }

    if (clt.do_write(msg, msglen) < 0) {
        printf("write error! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("write to server %s:%d buf: %s\n", server_ip, server_port, msg);
    }

    sleep(1);
    if (clt.reconnect(server_ip, server_port, -1) < 0) {
        printf("reconnect timeout! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("reconnect to server %s:%d success!\n", server_ip, server_port);
    }

    if (clt.do_read(read_buf, timeout_ms) < 0) {
        printf("read error! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("read from server %s:%d buf: %s\n", server_ip, server_port, read_buf);
    }
    if (clt.do_write(msg, msglen) < 0) {
        printf("write error! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("write to server %s:%d buf[%d]: %s\n", server_ip, server_port, msglen, msg);
    }

    if (clt.do_read(read_buf, timeout_ms) < 0) {
        printf("read error! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("read from server %s:%d buf: %s\n", server_ip, server_port, read_buf);
    }

    if (clt.do_write(msg, msglen) < 0) {
        printf("write error! %s\n", clt.get_errmsg());
        return -1;
    } else {
        printf("write to server %s:%d buf: %s\n", server_ip, server_port, msg);
    }

    sleep(1);
    sdsfree(read_buf);

    return 0;
}

#endif      // CLIENT_HELPER_TEST

}
