/*************************************************************************
	> File Name: logger.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月17日 星期二 01时56分06秒
 ************************************************************************/

#include "logger.h"
#include "file.h"
#include "common_tool.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace util {

int logger::vwrite_log(int level, const char *fmt, va_list ap)
{
    if (level <= LOG_OFF || level > LOG_FATAL) {
        set_errmsg(errmsg, "param error! level[%d]!", level);
        return ERR_PARAM;
    }

    if (log_level == LOG_OFF) {
        return 0;
    } else {
        if (level < log_level) {
            return 0;
        }
    }

    if (check_load()) {
        return 0;
    }

    datetime time;
    sds s = sdsnewlen(1024);
    if (s == NULL) {
        set_errmsg(errmsg, "sdsnewlen error!");
        return ERR_LIBCALL;
    }
    time.to_string(s);
    sdscatprintf(s, ".%d: ", time.get_usec());

    switch (level) {
    case LOG_DEBUG:
        sdscat(s, "[DEBUG] ");
        break;
    case LOG_INFO:
        sdscat(s, "[INFO] ");
        break;
    case LOG_WARN:
        sdscat(s, "[WARN] ");
        break;
    case LOG_ERROR:
        sdscat(s, "[ERROR] ");
        break;
    case LOG_FATAL:
        sdscat(s, "[FATAL] ");
        break;
    }

    file f(curr_name);
    if (f.open("a+") < 0) {
        set_errmsg(errmsg, "open error! %s", f.get_errmsg());
        return ERR_LIBCALL;
    }
    f.format("%s", s);
    f.vformat(fmt, ap);
    f.format("\n");
    ++write_num;

    sdsfree(s);
    return check_file();
}

int logger::check_load()
{
    if (max_write_num < 0) {
        return 0;
    }

    time_t t = last_log_time.timediff();
    if (t >= 1) {
        last_log_time.set_time();
        write_num = 0;
        return 0;
    }
    if (write_num >= max_write_num) {
        return 1;
    }

    return 0;
}

int logger::check_file()
{
    struct stat file_info;
    if (stat(curr_name, &file_info) < 0) {
        set_errmsg(errmsg, "stat %s error! errno[%d], %s", curr_name, errno, strerror(errno));
        return ERR_SYSCALL;
    }
    int file_size = file_info.st_size;

    if (file_size < one_file_size) {
        return 0;
    }

    char name[1024];
    sprintf(name, "%s%d.log", basename, max_file_num - 1);
    if (access(name, F_OK) == 0) {
        if (remove(name) < 0) {
            set_errmsg(errmsg, "remove %s error! errno[%d], %s", name, errno, strerror(errno));
            return ERR_SYSCALL;
        }
    }

    char new_name[1024];
    for (int i = max_file_num - 2; i >= 0; --i) {
        if (i == 0) {
            sprintf(name, "%s.log", basename);
        } else {
            sprintf(name, "%s%d.log", basename, i);
        }

        if (access(name, F_OK) == 0) {
            sprintf(new_name, "%s%d.log", basename, i + 1);
            if (rename(name, new_name) < 0) {
                printf("%d...\n", i);
                set_errmsg(errmsg, "rename %s to %s error! errno[%d], %s", name, new_name, errno, strerror(errno));
                return ERR_SYSCALL;
            }
        }
    }

    if (access(name, F_OK) < 0) {
        vsystem("touch %s", name);
    }

    return 0;
}

#ifdef LOGGER_TEST

int logger_test()
{
    logger log("test");

    log.set_file_size(10);
    log.set_file_num(4);
    log.set_load(3);
    log.debug("nihao");
    log.debug("nihao");
    log.debug("nihao");
    log.debug("nihao");

	return 0;
}

#endif      // LOGGER_TEST

}
