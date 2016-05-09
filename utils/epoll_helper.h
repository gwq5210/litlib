/*************************************************************************
	> File Name: epoll_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月25日 星期一 23时21分59秒
 ************************************************************************/

#ifndef _EPOLL_HELPER_H
#define _EPOLL_HELPER_H

#include <sys/epoll.h>

#include "sds.h"

namespace util {
/*
typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      // Epoll events
    epoll_data_t data;        // User data variable
};
*/

class epoll_helper {
public:
    epoll_helper();
    ~epoll_helper();
    int create(int size);
    int ctl(int op, int fd, int evt, long long usr_data);
    int add(int fd, int evt);
    int mod(int fd, int evt);
    int del(int fd, int evt);
    int wait(int timeout_ms);
    int get_events(int &evt, long long &usr_data);
    sds get_errmsg() { return errmsg; };
private:
    int epoll_fd;
    int max_fd_num;
    struct epoll_event *evts;
    int idx;
    int ready;
    sds errmsg;
};

#ifdef EPOLL_HELPER_TEST

int epoll_helper_test();

#endif      // EPOLL_HELPER_TEST

}

#endif      // _EPOLL_HELPER_H
