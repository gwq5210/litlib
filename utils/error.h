/*************************************************************************
	> File Name: error.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Sun Apr  3 15:44:45 2016
 ************************************************************************/

#ifndef _ERROR_H
#define _ERROR_H

namespace util {

enum ret_status {
    OK = 0,
    ERR_ALLOC = -1,
    ERR_PARAM = -2,
    ERR_DATA = -3,
    ERR_INIT = -4,
    ERR_NOINIT = -5,
    ERR_SETURL = -6,
};

}       // namespace util

#endif      // _ERROR_H
