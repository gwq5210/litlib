/*************************************************************************
	> File Name: test_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月26日 星期二 00时25分11秒
 ************************************************************************/

#ifndef _TEST_HELPER_H
#define _TEST_HELPER_H

#include "common_tool.h"

static int _test_num_ = 0;
static int _test_failed_num_ = 0;

#define test_cond(test_name, cond) do { \
    ++_test_num_; \
    printf("%d - %s: ", _test_num_, test_name); \
    if (cond) { \
        printf(GREEN"PASSED\n"RESET); \
    } else { \
        printf(RED"FAILED\n"RESET); \
        ++_test_failed_num_; \
    } \
} while(0)

#define test_report() do { \
    printf(GREEN"%d tests, %d passed, %d failed\n"RESET, _test_num_, \
        _test_num_ - _test_failed_num_, _test_failed_num_); \
    if (_test_failed_num_) { \
        printf(RED"=== WARNING === We have failed tests here...\n"RESET); \
    } \
} while(0)

#endif      // _TEST_HELPER_H
