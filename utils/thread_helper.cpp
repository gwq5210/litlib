/*************************************************************************
	> File Name: thread_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月09日 星期一 11时00分52秒
 ************************************************************************/

#include <unistd.h>

#include "jemalloc/jemalloc.h"
#include "thread_helper.h"
#include "mutex_helper.h"

namespace util {

int thread_helper::create_thread(thread_start_route start_route, void *arg, int thread_num/* = 1*/)
{
    if (start_route == NULL || thread_num <= 0) {
        set_errmsg(errmsg, "param error! start_route is NULL or thread_num less equal zero!");
        return ERR_PARAM;
    }

    thread_count = thread_num;
    thread_id = (pthread_t *)jemalloc(sizeof(pthread_t) * thread_count);
    if (thread_id == NULL) {
        set_errmsg(errmsg, "jemalloc error! thread_id is NULL!");
        return ERR_ALLOC;
    }
    for (int i = 0; i < thread_count; ++i) {
        int ret = pthread_create(&thread_id[i], &attr, start_route, arg);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_create %d error! errno[%d], %s", i, ret, strerror(ret));
            return ERR_LIBCALL;
        }
    }
    return 0;
}

int thread_helper::thread_join()
{
    if (thread_count == 0) {
        set_errmsg(errmsg, "thread not created! thread count is zero!");
        return ERR_PARAM;
    }

    ret_val = (void **)jemalloc(sizeof(void *) * thread_count);
    for (int i = 0; i < thread_count; ++i) {
        int ret = pthread_join(thread_id[i], &ret_val[i]);
        if (ret != 0) {
            set_errmsg(errmsg, "pthread_join %d error! errno[%d], %s", i, ret, strerror(ret));
            return ERR_LIBCALL;
        }
    }
    return 0;
}

#ifdef THREAD_HELPER_TEST

mutex_helper mutex;

void *dec_count(void *arg)
{
    int cnt = 10;
    while (cnt--) {
        usleep(100000);
        mutex.lock();
        *(int *)arg -= 1;
        printf("%d\n", *(int *)arg);
        mutex.unlock();
    }
    return NULL;
}

void *inc_count(void *arg)
{
    int cnt = 10;
    while (cnt--) {
        usleep(100000);
        mutex.lock();
        *(int *)arg += 1;
        printf("%d\n", *(int *)arg);
        mutex.unlock();
    }
    return NULL;
}

int thread_helper_test()
{
    int count = 0;
    thread_helper thread_dec;
    thread_helper thread_inc;
    thread_dec.create_thread(dec_count, &count, 10);
    thread_inc.create_thread(inc_count, &count, 10);
    thread_dec.thread_join();
    thread_inc.thread_join();


    //usleep(100000);
    printf("done!!! count = %d\n", count);
    return 0;
}

#endif      // THREAD_HELPER_TEST

}
