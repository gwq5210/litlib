/*************************************************************************
	> File Name: jemalloc_test.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Fri Feb 26 06:08:49 2016
 ************************************************************************/

#include <cstdio>
#include <cstring>

#include <jemalloc/jemalloc.h>

int main(int argc, char *argv[])
{
    char *s = (char *)malloc(1024);
    strcpy(s, "hello jemalloc!");
    printf("%s\n", s);
    free(s);
	return 0;
}
