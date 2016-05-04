/*************************************************************************
	> File Name: curl_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月24日 星期日 23时37分59秒
 ************************************************************************/

#ifndef _CURL_HELPER_H
#define _CURL_HELPER_H

#include "error.h"
#include "sds.h"
#include "common_tool.h"

#include "curl/curl.h"

namespace util {
class curl_helper {
public:
    curl_helper(): curl(NULL), rsp_buf(NULL), req_buf(NULL) { errmsg = sdsnewlen(1024); }
    ~curl_helper();
    int init();
    int cleanup();
    int perform(const char *url);
    int setopt(CURLoption option, void *option_val);
    int seturl(const char *url);
    const sds get_rsp() { return rsp_buf; }
    int set_req(const sds req)
    {
        if (sdscpysds(req_buf, req) == NULL) {
            set_errmsg(errmsg, "sdscpysds call error!");
            return ERR_LIBCALL;
        }
        return 0;
    }
    int set_req(const char *file_name)
    {
        sds s = load_file(file_name);
        if (s == NULL) {
            set_errmsg(errmsg, "load_file call error!");
            return ERR_LIBCALL;
        }
        int ret = set_req(s);
        if (s) {
            sdsfree(s);
        }
        return ret;
    }
    const sds get_errmsg() { return errmsg; }
    friend size_t proc_rsp(void *buf, size_t size, size_t num, void *arg);
    friend size_t proc_req(void *buf, size_t size, size_t num, void *arg);
private:
    CURL *curl;
    sds rsp_buf;
    sds req_buf;
    sds errmsg;
};

#ifdef CURL_HELPER_TEST

int curl_helper_test();

#endif // CURL_HELPER_TEST

}

#endif // _CURL_HELPER_H
