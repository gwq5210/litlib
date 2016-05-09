/*************************************************************************
	> File Name: curl_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月24日 星期日 23时45分57秒
 ************************************************************************/

#include "curl_helper.h"
#include "common_tool.h"

#include <string.h>

namespace util {

size_t proc_rsp(void *buf, size_t size, size_t num, void *arg)
{
    int real_size = size * num;
    curl_helper *ch = (curl_helper *)arg;
    ch->rsp_buf = sdscatlen(ch->rsp_buf, buf, real_size);
    return real_size;
}

size_t proc_req(void *buf, size_t size, size_t num, void *arg)
{
    curl_helper *ch = (curl_helper *)arg;
    size_t real_size = sdslen(ch->req_buf);
    memcpy(buf, ch->req_buf, real_size);
    return real_size;
}

curl_helper::~curl_helper()
{
    cleanup();
}

int curl_helper::init()
{
    CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
    if (code != CURLE_OK) {
        set_errmsg(errmsg, "curl_global_init call error! error code[%d], %s", code, curl_easy_strerror(code));
        return ERR_INIT;
    }
    curl = curl_easy_init();
    if (curl == NULL) {
        set_errmsg(errmsg, "curl_easy_init call error!");
        return ERR_INIT;
    }
    rsp_buf = sdsnewlen(SDS_MAX_PREMALLOC);
    if (rsp_buf == NULL) {
        set_errmsg(errmsg, "sdsnewlen call error! rsp_buf is NULL!");
        return ERR_INIT;
    }
    req_buf = sdsnewlen(SDS_MAX_PREMALLOC);
    if (req_buf == NULL) {
        set_errmsg(errmsg, "sdsnewlen call error! req_buf is NULL!");
        return ERR_INIT;
    }
    setopt(CURLOPT_WRITEFUNCTION, (void *)proc_rsp);
    setopt(CURLOPT_WRITEDATA, (void *)this);
    setopt(CURLOPT_READFUNCTION, (void *)proc_req);
    setopt(CURLOPT_READDATA, (void *)this);
    setopt(CURLOPT_FOLLOWLOCATION, (void *)1);
    return 0;
}

int curl_helper::cleanup()
{
    if (curl != NULL) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }
    sdsfree(rsp_buf);
    sdsfree(req_buf);
    sdsfree(errmsg);
    return 0;
}

int curl_helper::perform(const char *url)
{
    if (curl == NULL) {
        set_errmsg(errmsg, "curl is NULL!");
        return ERR_NOINIT;
    }
    int ret = setopt(CURLOPT_URL, (void *)url);
    if (ret < 0) {
        return ret;
    }
    CURLcode code = curl_easy_perform(curl);
    if (code != CURLE_OK) {
        set_errmsg(errmsg, "curl_easy_perform call error! error code[%d], %s", code, curl_easy_strerror(code));
        return ERR_LIBCALL;
    }
    return 0;
}

int curl_helper::setopt(CURLoption option, void * option_val)
{
    if (curl == NULL) {
        set_errmsg(errmsg, "curl is NULL!");
        return ERR_NOINIT;
    }
    CURLcode ret = curl_easy_setopt(curl, option, option_val);
    if (ret != CURLE_OK) {
        set_errmsg(errmsg, "curl_esay_setopt call error! error code[%d], %s", ret, curl_easy_strerror(ret));
        return ERR_LIBCALL;
    }
    return 0;
}

#ifdef CURL_HELPER_TEST

int curl_helper_test()
{
    const char *url = "www.baidu.com";
    curl_helper ch;
    int ret = ch.init();
    if (ret < 0) {
        printf("init error! %s\n", ch.get_errmsg());
        return ret;
    }
    printf("perform url %s...\n", url);
    ret = ch.perform(url);
    if (ret < 0) {
        printf("perform url(%s) error! %s\n", url, ch.get_errmsg());
    } else {
        printf("perform url %s complete!\n", url);
    }
    const sds s = ch.get_rsp();
    sdsprint(s);
    printf("rsp len = %d\n", sdslen(s));

    const char *file_name = "test.html";
    if (save_file(s, file_name) < 0) {
        printf("save file %s error!\n", file_name);
        return -1;
    } else {
        printf("save file %s success!\n", file_name);
    }
    return 0;
}

#endif // CURL_HELPER_TEST

}
