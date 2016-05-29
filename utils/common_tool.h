/*************************************************************************
	> File Name: common_tool.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月03日 星期二 14时04分52秒
 ************************************************************************/

#ifndef _COMMON_TOOL_H
#define _COMMON_TOOL_H

#include "sds.h"

//the following are UBUNTU/LINUX ONLY terminal color codes.
#define RESET "\033[0m"
#define BLACK "\033[30m" /* Black */
#define RED "\033[31m" /* Red */
#define GREEN "\033[32m" /* Green */
#define YELLOW "\033[33m" /* Yellow */
#define BLUE "\033[34m" /* Blue */
#define MAGENTA "\033[35m" /* Magenta */
#define CYAN "\033[36m" /* Cyan */
#define WHITE "\033[37m" /* White */
#define BOLDBLACK "\033[1m\033[30m" /* Bold Black */
#define BOLDRED "\033[1m\033[31m" /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m" /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m" /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m" /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m" /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m" /* Bold White */

#define set_errmsg(errmsg, fmt, ...) do { \
    sdsprintf(errmsg, "[%s:%s:%d]", __FILE__, __FUNCTION__, __LINE__); \
    sdscatprintf(errmsg, fmt, ##__VA_ARGS__); \
} while(0)

namespace util {

sds load_file(const char *file_name);
sds load_file(FILE *fp);
int save_file(const sds s, const char *file_name);
sds read_line(FILE *fp);
int socket_write(int socket_fd, const char *write_buf, int buflen, sds &errmsg);
int socket_read(int socket_fd, sds &read_buf, sds &errmsg, int timeout_ms = -1, int should_len = -1);
sds exec_cmd(const char *fmt, ...);
int vsystem(const char *fmt, ...);
int print_bin(const char *buf, int buflen = 0, FILE *fp = stdout);

#ifdef COMMON_TOOL_TEST

int common_tool_test();

#endif      // COMMON_TOOL_TEST

}

#endif      // _COMMON_TOOL_H
