/*************************************************************************
	> File Name: sds.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Sat 23 Apr 2016 07:23:27 PM CST
 ************************************************************************/

#ifndef _SDS_H
#define _SDS_H

#define SDS_MAGIC 0x4C8A6DE1
#define SDS_MAX_PREMALLOC (1024 * 1024)

typedef char *sds;

struct sds_header {
    int magic;
    int len;
    int free;
    char buf[];
};

sds sdsnew(const char *str);
sds sdsnew(const void *buf, int len);
sds sdsnew(const sds s);
int sdslen(const sds s);
int sdsavail(const sds s);
bool sdscheck(const sds s);
sds sdscpy(sds s, const char *str);
sds sdscpy(sds s, const void *buf, int buf_len);
sds sdscpy(sds dst, const sds src);
void sdsfree(sds s);
sds sdsgrow(sds s, int addlen);
void sdsprint(const sds s, FILE *fp = stdout);

#ifdef SDS_TEST
int sds_test(int argc, char *argv[]);
#endif      // SDS_TEST

#endif      // _SDS_H
