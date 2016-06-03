/*************************************************************************
	> File Name: mutex_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月09日 星期一 16时22分45秒
 ************************************************************************/

#ifndef _MUTEX_HELPER_H
#define _MUTEX_HELPER_H

#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "sds.h"
#include "common_tool.h"

namespace util {

class mutex_helper {
public:
    mutex_helper(const pthread_mutexattr_t *mutex_attr = NULL): errmsg(NULL) { init(mutex_attr); }
    ~mutex_helper() { clear(); }
    int lock()
    {
        int ret = pthread_mutex_lock(&mutex_lock);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_mutex_lock error! errno[%d], %s", ret, strerror(ret));
            return ERR_SYSCALL;
        }
        return 0;
    }
    int trylock()
    {
        int ret = pthread_mutex_trylock(&mutex_lock);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_mutex_trylock error! errno[%d], %s", ret, strerror(ret));
            return ERR_SYSCALL;
        }
        return 0;
    }
    int unlock()
    {
        int ret = pthread_mutex_unlock(&mutex_lock);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_mutex_unlock error! errno[%d], %s", ret, strerror(ret));
            return ERR_SYSCALL;
        }
        return 0;
    }
    const sds get_errmsg() { return errmsg; }
private:
    int init(const pthread_mutexattr_t *mutex_attr)
    {
        errmsg = sdsnewlen(1024);
        if (errmsg == NULL) {
            return ERR_LIBCALL;
        }
        int ret = pthread_mutex_init(&mutex_lock, mutex_attr);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_mutex_init error! errno[%d], %s", ret, strerror(ret));
            return ERR_SYSCALL;
        }
        return 0;
    }
    int clear()
    {
        int ret = pthread_mutex_destroy(&mutex_lock);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_mutex_destroy error! errno[%d], %s", ret, strerror(ret));
            return ERR_SYSCALL;
        }
        sdsfree(errmsg);
        return 0;
    }
private:
    pthread_mutex_t mutex_lock;
    sds errmsg;
};

#ifdef MUTEX_HELPER_TEST

int mutex_helper_test();

#endif      // MUTEX_HELPER_TEST

}


#endif      // _MUTEX_HELPER_H
