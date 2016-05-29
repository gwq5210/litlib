/*************************************************************************
	> File Name: file.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月17日 星期二 02时01分10秒
 ************************************************************************/

#include "file.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>

namespace util {

int file::open(const char *mode)
{
    if (filename == NULL) {
        set_errmsg(errmsg, "param error! filename is NULL!");
        return ERR_PARAM;
    }

    fp = fopen(filename, mode);
    if (fp == NULL) {
        set_errmsg(errmsg, "fopen %s error! errno[%d], %s", filename, errno, strerror(errno));
        return ERR_LIBCALL;
    }

    return 0;
}

sds file::read_all()
{
    if (fp == NULL) {
        set_errmsg(errmsg, "no init! fp is NULL!");
        return NULL;
    }

    if (setpos(0) < 0) {
        return NULL;
    }

    size_t size = file_size();
    if (size < 0) {
        return NULL;
    }

    char *buf = (char *)jemalloc(size);
    if (buf == NULL) {
        set_errmsg(errmsg, "jemalloc error! buf is NULL!");
        return NULL;
    }
    size_t read_num = fread(buf, size, 1, fp);
    if (read_num != 1) {
        jefree(buf);
        set_errmsg(errmsg, "fread error! read num[%d] is not 1!", read_num);
        return NULL;
    }
    sds s = sdsnewlen(buf, size);
    if (s == NULL) {
        jefree(buf);
        set_errmsg(errmsg, "sdsnewlne error! s is NULL!");
        return NULL;
    }
    jefree(buf);
    return s;
}

sds file::read_line()
{
    if (fp == NULL) {
        set_errmsg(errmsg, "no init! fp is NULL!");
        return NULL;
    }

    long cur_pos = getpos();
    if (cur_pos < 0) {
        return NULL;
    }

    char static_buf[1024];
    char *buf = static_buf;
    int buflen = sizeof(static_buf);

    buf[buflen - 2] = '\0';
    while (1) {
        if (fgets(buf, buflen, fp) == NULL) {
            if (buf != static_buf) {
                jefree(buf);
            }
            return NULL;
        }
        if (buf[buflen - 2] == '\0') {
            break;
        } else {
            if (setpos(cur_pos) < 0) {
                return NULL;
            }
            if (buf != static_buf) {
                jefree(buf);
            }
            buflen *= 2;
            buf = (char *)jemalloc(buflen);
            if (buf == NULL) {
                set_errmsg(errmsg, "jemalloc error! buf is NULL!");
                return NULL;
            }
            buf[buflen - 2] = '\0';
        }
    }

    sds s = sdsnew(buf);
    if (buf != static_buf) {
        jefree(buf);
    }
    sdstrim(s, "\n");
    return s;
}

int file::write(const char *buf, int buflen/* = 0*/)
{
    if (buf == NULL || buflen < 0) {
        set_errmsg(errmsg, "param error! buf is NULL or buflen less then zero!");
        return ERR_PARAM;
    }
    if (fp == NULL) {
        set_errmsg(errmsg, "no init! fp is NULL!");
        return ERR_PARAM;
    }

    if (buflen == 0) {
        buflen = strlen(buf);
    }
    if (fwrite(buf, 1, buflen, fp) != 1) {
        set_errmsg(errmsg, "fwrite file[%s] error! write num not 1!", filename);
        return ERR_LIBCALL;
    }
    return 0;
}

int file::format(const char *fmt, ...)
{
    if (fp == NULL) {
        set_errmsg(errmsg, "no init! fp is NULL!");
        return ERR_PARAM;
    }

    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(fp, fmt, ap);
    va_end(ap);
    return ret;
}

int file::vformat(const char *fmt, va_list ap)
{
    if (fp == NULL) {
        set_errmsg(errmsg, "no init! fp is NULL!");
        return ERR_PARAM;
    }

    return vfprintf(fp, fmt, ap);
}

size_t file::file_size()
{
    if (filename == NULL) {
        set_errmsg(errmsg, "param error! filename is NULL!");
        return ERR_PARAM;
    }

    struct stat file_info;
    if (access(filename, F_OK) < 0) {
        set_errmsg(errmsg, "access error! file[%s] not exist!", filename);
        return ERR_LIBCALL;
    }
    if (stat(filename, &file_info) < 0) {
        set_errmsg(errmsg, "stat file[%s] error! errno[%d], %s", filename, errno, strerror(errno));
        return ERR_LIBCALL;
    }
    size_t file_size = file_info.st_size;
    return file_size;
}

#ifdef FILE_TEST

int file_test()
{
    const char *name1 = "file_test1";
    file f1(name1);
    if (f1.open("w+") < 0) {
        printf("open error! %s\n", f1.get_errmsg());
        return -1;
    } else {
        printf("open file[%s] success!\n", name1);
    }
    f1.format("hello world! %d %.2f\n", 10, 2.3);
    sds s = f1.read_all();
    if (s == NULL) {
        printf("read all error! %s\n", f1.get_errmsg());
    } else {
        printf("read success!\n");
        sdsprint(s);
    }
    sdsfree(s);

    f1.setpos(0);
    while ((s = f1.read_line()) != NULL) {
        printf("%s\n", s);
        sdsfree(s);
    }

    return 0;
}

#endif      // FILE_TEST

}
