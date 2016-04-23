/*************************************************************************
	> File Name: lib_test.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Fri Feb 26 06:08:49 2016
 ************************************************************************/

#include <cstdio>
#include <cstring>

#include "sds.h"

int jemalloc_test(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    jemalloc_test(argc, argv);
    sds_test(argc, argv);
	return 0;
}
