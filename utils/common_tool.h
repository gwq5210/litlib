/*************************************************************************
	> File Name: common_tool.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月03日 星期二 14时04分52秒
 ************************************************************************/

#ifndef _COMMON_TOOL_
#define _COMMON_TOOL_

#include "sds.h"

#define set_errmsg(errmsg, fmt, ...) do { \
    sdsprintf(errmsg, "[%s:%s:%d]", __FILE__, __FUNCTION__, __LINE__); \
    sdscatprintf(errmsg, fmt, ##__VA_ARGS__); \
} while(0)

namespace util {

sds load_file(const char *file_name);
int save_file(const sds s, const char *file_name);
sds read_line(FILE *fp);

#ifdef COMMON_TOOL_TEST

int common_tool_test();

#endif      // COMMON_TOOL_TEST

}

#endif      // _COMMON_TOOL_
