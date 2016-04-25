/*************************************************************************
	> File Name: sds.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Sat 23 Apr 2016 07:22:25 PM CST
 ************************************************************************/

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "sds.h"
#include "error.h"

namespace util {

sds sdsnew(const char *str/* = NULL */)
{
    return sdsnew(str, strlen(str));
}

sds sdsnew(int len)
{
    sds s = sdsnew(NULL, len);
    sdsclear(s);
    return s;
}

sds sdsnew(const void *buf, int len)
{
    if (len < 0) {
        return NULL;
    }

    int alloc_len = sizeof(sds_header) + len + 1;
    sds_header *sh = (sds_header *)malloc(alloc_len);
    if (sh == NULL) {
        return NULL;
    }

    memset(sh, 0, alloc_len);
    sh->magic = SDS_MAGIC;
    sh->len = len;
    sh->free = 0;

    if (buf && len) {
        memcpy(sh->buf, buf, len);
    }
    sh->buf[len] = '\0';

    return sh->buf;
}

sds sdsnew(const sds s)
{
    if (!sdscheck(s)) {
        return NULL;
    }
    return sdsnew(s, sdslen(s));
}

inline int sdslen(const sds s)
{
    if (!sdscheck(s)) {
        return ERR_DATA;
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    return sh->len;
}

inline int sdsavail(const sds s)
{
    if (!sdscheck(s)) {
        return ERR_DATA;
    }
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

sds sdscpy(sds s, const char *str)
{
    return sdscpy(s, str, strlen(str));
}

sds sdscpy(sds s, const void *buf, int buf_len)
{
    int totlen = sdslen(s) + sdsavail(s);
    if (totlen < buf_len) {
        s = sdsgrow(s, buf_len - sdslen(s));
        if (s == NULL) {
            return NULL;
        }
        totlen = sdslen(s) + sdsavail(s);
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    sh->magic = SDS_MAGIC;
    memcpy(sh->buf, buf, buf_len);
    sh->buf[buf_len] = '\0';
    sh->len = buf_len;
    sh->free = totlen - buf_len;
    return s;
}

sds sdscpy(sds dst, const sds src)
{
    return sdscpy(dst, src, sdslen(src));
}

sds sdscat(sds s, const void *buf, int buf_len)
{
    if (!sdscheck(s)) {
        return NULL;
    }
    int len = sdslen(s);
    int free = sdsavail(s);
    if (free < buf_len) {
        s = sdsgrow(s, buf_len);
        if (s == NULL) {
            return NULL;
        }
        free = sdsavail(s);
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    memcpy(s + len, buf, buf_len);
    sh->len += buf_len;
    sh->free -= buf_len;
    s[sh->len] = '\0';
    return s;
}

sds sdscat(sds s, const char *str)
{
    return sdscat(s, str, strlen(str));
}

sds sdscat(sds dst, const sds src)
{
    return sdscat(dst, src, sdslen(src));
}

void sdsclear(sds s)
{
    if (!sdscheck(s)) {
        return;
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    sh->free += sh->len;
    sh->len = 0;
    s[0] = '\0';
}

void sdsfree(sds s)
{
    if (!sdscheck(s)) {
        return;
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    free(sh);
}

sds sdsgrow(sds s, int addlen)
{
    if (!sdscheck(s)) {
        return NULL;
    }
    int free = sdsavail(s);
    if (free >= addlen) {
        return s;
    }
    int len = sdslen(s);
    int new_tot_len = len + addlen;
    if (len < SDS_MAX_PREMALLOC) {
        new_tot_len *= 2;
    } else {
        new_tot_len += SDS_MAX_PREMALLOC;
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    sh = (sds_header *)realloc(sh, sizeof(sds_header) + new_tot_len + 1);
    if (!s) {
        return NULL;
    }
    sh->free = new_tot_len - len;
    return sh->buf;
}

void sdsprint(const sds s, FILE *fp/* = stdout */)
{
    if (!sdscheck(s)) {
        fprintf(fp, "sds error!");
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    fprintf(fp, "sh->magic = %X\n", sh->magic);
    fprintf(fp, "sh->len = %d\n", sh->len);
    fprintf(fp, "sh->free = %d\n", sh->free);
    if (sh->len == 0) {
        fprintf(fp, "sh->buf is empty!\n");
        return;
    }
    fprintf(fp, "s->buf:\n");
    int col = 10;
    int row = sh->len / col + (sh->len % col ? 1 : 0);
    for (int i = 0; i < row; ++i) {
        fprintf(fp, "%010X: ", i);
        for (int j = 0; j < col; ++j) {
            int cnt = i * col + j;
            if (cnt >= sh->len) {
                fprintf(fp, "  ");
            } else {
                fprintf(fp, "%02X", s[cnt]);
            }
            fprintf(fp, "%s", j == col - 1 ? "\t": " ");
        }
        for (int j = 0; j < col; ++j) {
            int cnt = i * col + j;
            if (cnt >= sh->len) {
                fprintf(fp, " ");
            } else {
                fprintf(fp, "%c", isprint(s[cnt]) ? s[cnt] : '.');
            }
        }
        fprintf(fp, "\n");
    }
}

#ifdef SDS_TEST

int sds_test(int argc, char *argv[])
{
    sds s = sdsnew("nihao______fdajfkdsajfklj\0nifdhf", 32);        // 32
    sdsprint(s);
    s = sdscat(s, "nihao", 5);
    s = sdscat(s, "nih\t\nao", 7);
    sdsprint(s);
    sdsfree(s);
    return 0;
}

#endif      // SDS_TEST

}       // namespace util
