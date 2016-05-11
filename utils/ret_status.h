/*************************************************************************
	> File Name: ret_status.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Sun Apr  3 15:44:45 2016
 ************************************************************************/

#ifndef _RET_STATUS_H
#define _RET_STATUS_H

namespace util {

enum ret_status {
    OK = 0,
    ERR_ALLOC = -1,
    ERR_PARAM = -2,
    ERR_DATA = -3,
    ERR_INIT = -4,
    ERR_NOINIT = -5,
    ERR_SYSCALL = -6,
    ERR_NODATA = -7,
    ERR_LIBCALL = -8,
    ERR_EAGAIN = -9,
    ERR_REG_NOMATCH = -10,
    ERR_TIMEOUT = -11,
    ERR_EMPTY = -12,
};

}       // namespace util

#endif      // _RET_STATUS_H
