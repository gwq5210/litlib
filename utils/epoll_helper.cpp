/*************************************************************************
	> File Name: epoll_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月25日 星期一 23时21分57秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "error.h"
#include "epoll_helper.h"
#include "common_tool.h"

namespace util {

epoll_helper::epoll_helper()
{
    epoll_fd = -1;
    evts = NULL;
    idx = 0;
    ready = 0;
    errmsg = sdsnewlen(1024);
}

epoll_helper::~epoll_helper()
{
    if (epoll_fd >= 0) {
        close(epoll_fd);
    }
    if (evts != NULL) {
        delete []evts;
    }
    sdsfree(errmsg);
}

int epoll_helper::create(int max_num)
{
    max_fd_num = max_num;
    evts = new struct epoll_event[max_fd_num];
    epoll_fd = epoll_create(max_num);
    if (epoll_fd < 0) {
        set_errmsg(errmsg, "epoll_create call error! errno[%d], %s", errno, strerror(errno));
        return ERR_SYSCALL;
    }
    return 0;
}

int epoll_helper::ctl(int op, int fd, int evt, long long usr_data)
{
    struct epoll_event e;
    e.events = evt;
    e.data.u64 = usr_data;
    if (epoll_fd < 0) {
        set_errmsg(errmsg, "epoll fd[%d] is less then zero!", epoll_fd);
        return ERR_NOINIT;
    }
    if (evts == NULL) {
        set_errmsg(errmsg, "evts is NULL!");
        return ERR_NOINIT;
    }
    int ret = epoll_ctl(epoll_fd, op, fd, &e);
    if (ret < 0) {
        set_errmsg(errmsg, "epoll_ctl call error! errno[%d], %s", errno, strerror(errno));
        return ERR_SYSCALL;
    }
    return 0;
}

int epoll_helper::add(int fd, int evt)
{
    return ctl(EPOLL_CTL_ADD, fd, evt, fd);
}

int epoll_helper::mod(int fd, int evt)
{
    return ctl(EPOLL_CTL_MOD, fd, evt, fd);
}

int epoll_helper::del(int fd, int evt)
{
    return ctl(EPOLL_CTL_DEL, fd, evt, fd);
}

int epoll_helper::wait(int timeout_ms)
{
    if (epoll_fd < 0) {
        set_errmsg(errmsg, "epoll fd[%d] is less then zero!", epoll_fd);
        return ERR_NOINIT;
    }
    if (evts == NULL) {
        set_errmsg(errmsg, "evts is NULL!");
        return ERR_NOINIT;
    }
    int ret = epoll_wait(epoll_fd, evts, max_fd_num, timeout_ms);
    if (ret < 0) {
        set_errmsg(errmsg, "epoll_wait call error! errno[%d], %s", errno, strerror(errno));
        return ERR_SYSCALL;
    }
    ready = ret;
    idx = 0;
    return ret;
}

int epoll_helper::get_events(int &evt, long long &usr_data)
{
    if (evts == NULL) {
        set_errmsg(errmsg, "evts is NULL!");
        return ERR_NOINIT;
    }
    if (idx >= ready) {
        set_errmsg(errmsg, "idx[%d] greater then ready event num[%d]!", idx, ready);
        return ERR_NODATA;
    }

    struct epoll_event &e = evts[idx++];
    evt = e.events;
    usr_data = e.data.u64;
    return 0;
}

#ifdef EPOLL_HELPER_TEST

int epoll_helper_test()
{
    epoll_helper eh;
    if (eh.create(16) < 0) {
        printf("epoll_helper create error! %s", eh.get_errmsg());
        return -1;
    }
    eh.add(STDIN_FILENO, EPOLLIN);
    int timeout_ms = 1000;
    int ret = eh.wait(timeout_ms);
    if (ret <= 0) {
        printf("no fd ready! %s\n", eh.get_errmsg());
        return -1;
    } else {
        printf("wait %dms done!\n", timeout_ms);
    }

    int evt = 0;
    long long fd = -1;
    while (eh.get_events(evt, fd) == 0) {
        if (EPOLLIN & evt) {
            printf("fd %lld has data to read!\n", fd);
        } else {
            printf("no data!\n");
        }
    }
    return 0;
}

#endif      // EPOLL_HELPER_TEST

}
