/*************************************************************************
	> File Name: curl_tool.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月24日 星期日 23时45分57秒
 ************************************************************************/

#include "curl_tool.h"

namespace util {

size_t proc_rsp(void *buf, size_t size, size_t num, void *arg)
{
    int real_size = size * num;
    curl_tool *ct = (curl_tool *)arg;
    ct->rsp = sdscat(ct->rsp, buf, real_size);
    return real_size;
}

curl_tool::~curl_tool()
{
    cleanup();
}

int curl_tool::init()
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl == NULL) {
        return ERR_INIT;
    }
    rsp = sdsnew(SDS_MAX_PREMALLOC);
    if (rsp == NULL) {
        return ERR_INIT;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, proc_rsp);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    return 0;
}

int curl_tool::cleanup()
{
    if (curl != NULL) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }
    if (rsp != NULL) {
        sdsfree(rsp);
    }
    return 0;
}

int curl_tool::perform(const char *url)
{
    int ret = seturl(url);
    if (ret < 0) {
        return ERR_SETURL;
    }
    curl_easy_perform(curl);
    return 0;
}

int curl_tool::setopt(CURLoption op, void *val)
{
    return 0;
}

int curl_tool::seturl(const char *url)
{
    if (curl == NULL) {
        return ERR_NOINIT;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    printf("set url = %s\n", url);
    return 0;
}

const sds curl_tool::get_rsp()
{
    return rsp;
}

#ifdef CURL_TOOL_TEST

int curl_tool_test(int argc, char *argv[])
{
    const char *url = "www.baidu.com";
    curl_tool ct;
    int ret = ct.init();
    if (ret < 0) {
        printf("init error! ret = %d", ret);
        return ret;
    }
    printf("perform url %s...\n", url);
    ret = ct.perform(url);
    if (ret < 0) {
        printf("perform url(%s) error! ret = %d\n", url, ret);
    }
    printf("perform url %s complete!\n", url);
    sds s = ct.get_rsp();
    sdsprint(s);
    printf("%s\n", s);
    return 0;
}

#endif // CURL_TOOL_TEST

}
