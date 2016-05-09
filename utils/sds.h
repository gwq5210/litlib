/*************************************************************************
	> File Name: sds.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Sat 23 Apr 2016 07:23:27 PM CST
 ************************************************************************/

#ifndef _SDS_H
#define _SDS_H

#include <stdio.h>
#include <stdarg.h>

#include "jemalloc/jemalloc.h"

namespace util {

#define SDS_MAGIC 0x4C8A6DE1
#define SDS_MAX_PREMALLOC (1024 * 1024)

#define SPLIT_ALL 0
#define SPLIT_FIRST 1
#define SPLIT_LAST 2

typedef char *sds;

struct sds_header {
    int magic;
    int len;
    int free;
    char buf[];
};

sds sdsnew(const char *str = NULL);
sds sdsnewlen(int len);
sds sdsnewlen(const void *buf, int len);
sds sdsnewsds(const sds s);
sds sdscpy(sds &s, const char *str);
sds sdscpylen(sds &s, const void *buf, int buf_len);
sds sdscpysds(sds &dst, const sds src);
sds sdscatlen(sds &s, const void *buf, int buf_len);
sds sdscat(sds &s, const char *str);
sds sdscatsds(sds &dst, const sds src);
sds sdstrim(sds &s, const char *cset = " \t\n");
sds sdsltrim(sds &s, const char *cset = " \t\n");
sds sdsrtrim(sds &s, const char *cset = " \t\n");
sds *sdssplit(const sds s, const char *sep, int &count, int split_type = SPLIT_ALL);
sds *sdssplitlen(const sds s, const sds sep, int &count, int split_type = SPLIT_ALL);
sds sdstoupper(sds s);
sds sdstolower(sds s);
int sdssplitfree(sds *&s, int count);
void sdsclear(sds s);
void sdsfree(sds &s);
sds sdsgrow(sds &s, int addlen);
void sdsprint(const sds s, FILE *fp = stdout);
int sdsprintf(sds &s, const char *fmt, ...);
int vsdsprintf(sds &s, const char *fmt, va_list ap);
int sdscatprintf(sds &s, const char *fmt, ...);
int vsdscatprintf(sds &s, const char *fmt, va_list ap);

inline int sdslen(const sds s)
{
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    return sh->len;
}

inline int sdsavail(const sds s)
{
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    return sh->free;
}

inline bool sdscheck(const sds s)
{
    if (!s) {
        return false;
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    if (sh->magic != SDS_MAGIC) {
        return false;
    }
    if (sh->len < 0 || sh->free < 0) {
        return false;
    }

    return true;
}

inline bool sdsempty(const sds s)
{
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    return sh->len == 0;
}

#ifdef SDS_TEST

int sds_test();

#endif      // SDS_TEST

}       // namespace util

#endif      // _SDS_H
