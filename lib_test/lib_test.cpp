/*************************************************************************
	> File Name: lib_test.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Fri Feb 26 06:08:49 2016
 ************************************************************************/

#include <stdio.h>
#include <cstring>

#include "sds.h"
#include "curl_helper.h"
#include "epoll_helper.h"
#include "test_helper.h"
#include "mysql_helper.h"
#include "cfg_helper.h"

using namespace std;
using namespace util;

int jemalloc_test();
int all_test();

const static int test_num = 8;
char test_name[test_num][1024] = {"all", "jemalloc", "sds", "curl_helper", "epoll_helper", "common_tool",
    "mysql_helper", "cfg_helper"};
int (*test_func[test_num])() = {all_test, jemalloc_test, sds_test, curl_helper_test, epoll_helper_test,
    common_tool_test, mysql_helper_test, cfg_helper_test};

int all_test()
{
    for (int i = 1; i < test_num; ++i) {
        printf(GREEN"---- %s test begin ----\n"RESET, test_name[i]);
        test_func[i]();
        printf(GREEN"---- %s test end ----\n"RESET, test_name[i]);
    }
    return 0;
}

void print_usage(char *name)
{
    printf("Usage: %s test_name0 test_name1 ...\n", name);
    printf("test num %d!\n", test_num);
    printf("test name: ");
    for (int i = 0; i < test_num; ++i) {
        printf("%s%s", test_name[i], i == test_num - 1 ? "\n" : " "); 
    }
}

int main(int argc, char *argv[])
{
    int flag = 1;
    for (int i = 1; i < argc; ++i) {
        const char *name = argv[i];
        int has = -1;
        for (int j = 0; j < test_num; ++j) {
            if (strcmp(name, test_name[j]) == 0) {
                has = j;
                break;
            }
        }
        if (has > 0) {
            printf(GREEN"---- %s test begin ----\n"RESET, name);
            test_func[has]();
            printf(GREEN"---- %s test end ----\n"RESET, name);
            flag = 0;
        } else if (has == 0) {
            test_func[has]();
        } else {
            printf(RED"%s test had no found!\n"RESET, name);
        }
    }
    if (flag) {
        print_usage(argv[0]);
    }
	return 0;
}
