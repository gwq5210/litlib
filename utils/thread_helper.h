/*************************************************************************
	> File Name: thread_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月09日 星期一 11时01分20秒
 ************************************************************************/

#ifndef _THREAD_HELPER_H
#define _THREAD_HELPER_H

#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "ret_status.h"
#include "sds.h"
#include "common_tool.h"

namespace util {

typedef void *(*thread_start_route)(void *);

class thread_helper {
public:
    thread_helper(): thread_id(NULL), thread_count(0)
    {
        init();
    }
    ~thread_helper() { clear(); }
    int init()
    {
        int ret = pthread_attr_init(&attr);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_attr_init error! errno[%d], %s", ret, strerror(ret));
            return ERR_INIT;
        }
        errmsg = sdsnewlen(1024);
        if (errmsg == NULL) {
            set_errmsg(errmsg, "sdsnewlen error! errmsg is NULL!");
            return ERR_LIBCALL;
        }
        return 0;
    }
    int clear()
    {
        int ret = pthread_attr_destroy(&attr);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_attr_destroy error! errno[%d], %s", ret, strerror(ret));
            return ERR_LIBCALL;
        }
        if (thread_id != NULL) {
            jefree(thread_id);
            thread_id = NULL;
        }
        if (ret_val != NULL) {
            jefree(ret_val);
            ret_val = NULL;
        }
        sdsfree(errmsg);
        return 0;
    }
    int create_thread(thread_start_route start_route, void *arg, int thread_num = 1);
    const sds get_errmsg() { return errmsg; }
    pthread_attr_t *get_thread_attr() { return &attr; }
    int thread_join();
    void *get_ret_val(int count)
    {
        if (thread_count == 0) {
            set_errmsg(errmsg, "thread not created! thread count is zero!");
            return NULL;
        }
        if (count < 0 || count >= count) {
            set_errmsg(errmsg, "param error! count[%d] less then zero or count[%d] greater equal thread_count[%d]", count, count, thread_count);
            return NULL;
        }
        if (ret_val == NULL) {
            set_errmsg(errmsg, "ret val is NULL!");
            return NULL;
        }
        return ret_val[count];
    }
private:
    pthread_t *thread_id;
    void **ret_val;
    int thread_count;
    pthread_attr_t attr;
    sds errmsg;
};

#ifdef THREAD_HELPER_TEST

int thread_helper_test();

#endif      // THREAD_HELPER_TEST

}

#endif      // _THREAD_HELPER_H
