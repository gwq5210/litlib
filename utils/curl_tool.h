/*************************************************************************
	> File Name: curl_tool.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月24日 星期日 23时37分59秒
 ************************************************************************/

#ifndef _CURL_TOOL_H
#define _CURL_TOOL_H

#include "error.h"
#include "sds.h"

#include "curl/curl.h"

namespace util {
class curl_tool {
public:
    curl_tool(): curl(NULL), rsp(NULL) {};
    ~curl_tool();
    int init();
    int cleanup();
    int perform(const char *url);
    int setopt(CURLoption op, void *val);
    int seturl(const char *url);
    const sds get_rsp();
    friend size_t proc_rsp(void *buf, size_t size, size_t num, void *arg);
private:
    CURL *curl;
    sds rsp;
};

#ifdef CURL_TOOL_TEST

int curl_tool_test(int argc, char *argv[]);

#endif // CURL_TOOL_TEST

}

#endif // _CURL_TOOL_H
