/*************************************************************************
	> File Name: test_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月26日 星期二 00时25分11秒
 ************************************************************************/

#ifndef _TEST_HELPER_
#define _TEST_HELPER_

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

int _test_num_ = 0;
int _test_failed_num_ = 0;

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

#endif      // _TEST_HELPER_
