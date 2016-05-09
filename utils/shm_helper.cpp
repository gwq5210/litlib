/*************************************************************************
	> File Name: shm_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月09日 星期一 21时06分24秒
 ************************************************************************/

#include "shm_helper.h"

namespace util {

int shm_helper::create(key_t key, int size, int create_type/* = SHM_ATTACH*/)
{
    if (size <= 0) {
        set_errmsg(errmsg, "param error! size less equal zero!");
        return ERR_PARAM;
    }
    if (create_type != SHM_CREATE && create_type != SHM_ATTACH && create_type != SHM_EXCL) {
        set_errmsg(errmsg, "param error! create_type[%d] is illegal!", create_type);
        return ERR_PARAM;
    }

    shm_key = key;
    shm_size = size;

    shm_id = shmget(shm_key, shm_size, 0666 | IPC_CREAT | IPC_EXCL);
    if ((shm_id < 0 && errno != EEXIST) || (shm_id < 0 && create_type == SHM_EXCL)) {
        set_errmsg(errmsg, "shmget error! shm key[%d], shm size[%d], errno[%d], %s", shm_key, shm_size, errno, strerror(errno));
        return ERR_LIBCALL;
    } else if (shm_id < 0 && errno == EEXIST) {
        if (create_type == SHM_CREATE) {
            shm_id = shmget(shm_key, 0, 0666);
        } else if (create_type == SHM_ATTACH){
            shm_id = shmget(shm_key, shm_size, 0666);
        }
        if (shm_id < 0) {
            set_errmsg(errmsg, "shmget error! shm key[%d], shm size[%d], errno[%d], %s", shm_key, shm_size, errno, strerror(errno));
            return ERR_LIBCALL;
        }

        if (create_type == SHM_CREATE) {
            if (del_shm() < 0) {
                return ERR_LIBCALL;
            }
            shm_id = shmget(shm_key, shm_size, 0666 | IPC_CREAT | IPC_EXCL);
            if (shm_id < 0) {
                set_errmsg(errmsg, "shmget error! shm key[%d], shm size[%d], errno[%d], %s", shm_key, shm_size, errno, strerror(errno));
                return ERR_LIBCALL;
            }
        }
    }

    shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        set_errmsg(errmsg, "shmat error! shm key[%d], shm id[%d], shm size[%d], errno[%d], %s", shm_key, shm_id, shm_size, errno, strerror(errno));
        return ERR_LIBCALL;
    }
    return 0;
}

int shm_helper::del_shm()
{
    if (shm_id < 0) {
        set_errmsg(errmsg, "shm id is less then zero!");
        return ERR_NOINIT;
    }

    struct shmid_ds ds;
    if (ctl_shm(IPC_STAT, &ds) < 0) {
        return ERR_SYSCALL;
    }

    if (ds.shm_nattch < 0) {
        set_errmsg(errmsg, "shm id[%d], shm_key[%d], shm size[%d] del shm error! shm attach[%d] greater zero!", shm_id, shm_key, shm_size, ds.shm_nattch);
        return ERR_LIBCALL;
    }

    if (ctl_shm(IPC_RMID, NULL) < 0) {
        return ERR_SYSCALL;
    }

    return 0;
}

int shm_helper::ctl_shm(int cmd, struct shmid_ds *buf)
{
    if (shm_id < 0) {
        set_errmsg(errmsg, "shm id is less then zero!");
        return ERR_NOINIT;
    }

    if (shmctl(shm_id, cmd, buf) < 0) {
        set_errmsg(errmsg, "shmctl error! shm id[%d], cmd[%d], errno[%d], %s", shm_id, cmd, errno, strerror(errno));
        return ERR_SYSCALL;
    }
    return 0;
}

#ifdef SHM_HELPER_TEST

int shm_helper_test()
{
    int shm_key = 0x12345;
    int shm_size = 1024;
    shm_helper shm;
    if (shm.create(shm_key, shm_size) < 0) {
        printf("create shm error! %s\n", shm.get_errmsg());
        return -1;
    } else {
        printf("create shm success! shm key[%#x], shm id[%d], shm size[%d]\n", shm_key, shm.get_shm_id(), shm_size);
    }

    char *buf = (char *)shm.get_shm();
    strcpy(buf, "nihao");

    int new_shm_key = 0x12345;
    int new_shm_size = shm_size + 10;
    shm_helper new_shm;
    if (new_shm.create(shm_key, new_shm_size, SHM_CREATE) < 0) {
        printf("create new shm error! %s\n", new_shm.get_errmsg());
        return -1;
    } else {
        printf("create new shm success! new shm key[%#x], new shm id[%d], new shm size[%d]\n", new_shm_key, new_shm.get_shm_id(), new_shm_size);
    }
    if (new_shm.del_shm() < 0) {
        printf("del shm error! %s\n", new_shm.get_errmsg());
        return -1;
    } else {
        printf("del shm success!\n");
    }

    return 0;
}

#endif      // SHM_HELPER_TEST

}
