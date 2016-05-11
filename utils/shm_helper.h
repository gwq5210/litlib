/*************************************************************************
	> File Name: shm_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月09日 星期一 21时06分30秒
 ************************************************************************/

#ifndef _SHM_HELPER_H
#define _SHM_HELPER_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>

#include "sds.h"
#include "ret_status.h"
#include "common_tool.h"

#define SHM_ATTACH 0
#define SHM_CREATE 1
#define SHM_EXCL 2

namespace util {

class shm_helper {
public:
    shm_helper(): shm_key(-1), shm_id(-1), shm_size(-1), shm_ptr(NULL), errmsg(NULL) { init(); }
    ~shm_helper() { clear(); }
    int init()
    {
        errmsg = sdsnewlen(1024);
        if (errmsg == NULL) {
            return ERR_LIBCALL;
        }
        return 0;
    }
    int clear()
    {
        if (shm_ptr != NULL) {
            if (shmdt(shm_ptr) < 0) {
                set_errmsg(errmsg, "shmdt error! shm id[%d], errno[%d], %s", shm_id, errno, strerror(errno));
                return ERR_SYSCALL;
            }
        }
        shm_key = -1;
        shm_id = -1;
        shm_size = -1;
        sdsfree(errmsg);
        return 0;
    }
    int create(key_t key, int size, int create_type = SHM_ATTACH);
    int del_shm();
    int ctl_shm(int cmd, struct shmid_ds *buf);
    void *get_shm() { return shm_ptr; }
    const sds get_errmsg() { return errmsg; }
    int get_shm_id() { return shm_id; }
private:
    key_t shm_key;
    int shm_id;
    int shm_size;
    void *shm_ptr;
    sds errmsg;
};

#ifdef SHM_HELPER_TEST

int shm_helper_test();

#endif      // SHM_HELPER_TEST

}

#endif      // _SHM_HELPER_H
