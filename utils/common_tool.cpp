/*************************************************************************
	> File Name: common_tool.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月03日 星期二 14时04分40秒
 ************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "common_tool.h"
#include "jemalloc/jemalloc.h"

#include "error.h"

namespace util {

sds load_file(const char *file_name)
{
    struct stat file_info;
    if (access(file_name, F_OK) < 0) {
        return NULL;
    }
    if (stat(file_name, &file_info) < 0) {
        return NULL;
    }
    size_t file_size = file_info.st_size;
    FILE *fp = fopen(file_name, "rb");
    if (fp == NULL) {
        return NULL;
    }
    char *buf = (char *)jemalloc(file_size);
    if (buf == NULL) {
        return NULL;
    }
    size_t read_num = fread(buf, file_size, 1, fp);
    if (read_num != 1) {
        jefree(buf);
        return NULL;
    }
    sds s = sdsnewlen(buf, file_size);
    if (s == NULL) {
        jefree(buf);
        return NULL;
    }
    jefree(buf);
    fclose(fp);
    return s;
}

int save_file(const sds s, const char *file_name)
{
    FILE *fp = fopen(file_name, "wb");
    if (fp == NULL) {
        return ERR_SYSCALL;
    }

    int file_size = sdslen(s);
    int write_num = fwrite(s, file_size, 1, fp);
    if (write_num != 1) {
        return ERR_SYSCALL;
    }
    fclose(fp);
    return 0;
}

sds read_line(FILE *fp)
{
    if (fp == NULL) {
        return NULL;
    }

    long cur_pos = ftell(fp);
    if (cur_pos < 0) {
        return NULL;
    }

    char static_buf[5];
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
            if (fseek(fp, cur_pos, SEEK_SET) < 0) {
                return NULL;
            }
            if (buf != static_buf) {
                jefree(buf);
            }
            buflen *= 2;
            buf = (char *)jemalloc(buflen);
            if (buf == NULL) {
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

#ifdef COMMON_TOOL_TEST

int common_tool_test()
{
    sds s = load_file(__FILE__);
    if (s == NULL) { printf("load_file %s error! errno[%d], %s\n", __FILE__, errno, strerror(errno));
        return -1;
    }
    sdsprint(s);
    sdsfree(s);

    const char *file_name = "test_file";
    s = sdsnew("test_file");
    if (save_file(s, file_name) < 0) {
        printf("save file %s error!\n", file_name);
        return -1;
    } else {
        printf("save file %s success!\n", file_name);
    }
    sdsfree(s);

    FILE *fp = fopen(__FILE__, "r");
    while ((s = read_line(fp)) != NULL) {
        printf("%s\n", s);
        sdsfree(s);
    }
    fclose(fp);
    return 0;
}

#endif      // COMMON_TOOL_TEST

}
