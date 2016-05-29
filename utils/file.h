/*************************************************************************
	> File Name: file.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月17日 星期二 02时01分15秒
 ************************************************************************/

#ifndef _FILE_H
#define _FILE_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "sds.h"
#include "common_tool.h"
#include "ret_status.h"

namespace util {

class file {
public:
    file(const char *name): fp(NULL), filename(NULL), errmsg(NULL)
    {
        errmsg = sdsnewlen(1024);
        filename = sdsnew(name);
    }
    ~file()
    {
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        sdsfree(filename);
        sdsfree(errmsg);
    }
    int open(const char *mode);
    sds read_all();
    sds read_line();
    int seek(int offset, int whence)
    {
        if (fp == NULL) {
            set_errmsg(errmsg, "no init! fp is NULL!");
            return ERR_NOINIT;
        }

        if (fseek(fp, offset, whence) < 0) {
            set_errmsg(errmsg, "fseek file[%s] error! errno[%d], %s", filename, errno, strerror(errno));
            return ERR_LIBCALL;
        }
        return 0;
    }
    int setpos(long pos)
    {
        return seek(pos, SEEK_SET);
    }
    long getpos()
    {
        if (fp == NULL) {
            set_errmsg(errmsg, "no init! fp is NULL!");
            return ERR_NOINIT;
        }

        long pos = ftell(fp);
        if (pos < 0) {
            set_errmsg(errmsg, "ftell file[%s] error! errno[%d], %s", filename, errno, strerror(errno));
        }
        return pos;
    }
    size_t file_size();
    int write(const char *buf, int buflen = 0);
    int format(const char *fmt, ...);
    int vformat(const char *fmt, va_list ap);
    const sds get_errmsg() { return errmsg; }
    const sds get_filename() { return filename; }
private:
    FILE *fp;
    sds filename;
    sds errmsg;
};

#ifdef FILE_TEST

int file_test();

#endif      // FILE_TEST

}

#endif      // _FILE_H
