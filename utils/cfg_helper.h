/*************************************************************************
	> File Name: cfg_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月25日 星期一 23时22分23秒
 ************************************************************************/

#ifndef _CFG_HELPER_
#define _CFG_HELPER_

#include <map>
#include <string>

#include "error.h"
#include "sds.h"

#define CFG_NO_INT INT_MAX
#define CFG_NO_DBL DBL_MAX

namespace util {

class cfg_helper {
public:
    cfg_helper() { errmsg = sdsnewlen(1024); }
    ~cfg_helper()
    {
        cfg_map.clear();
        sdsfree(errmsg);
    }
    int get_int(const char *cfg_name, int default_val = -1);
    sds get_str(const char *cfg_name, const char *default_val = "invalid");
    double get_dbl(const char *cfg_name, double default_val = -1.0);
    int read_cfg(const char *cfg_file_name);
    bool is_have(const char *cfg_name);
    void print(FILE *fp = stdout)
    {
        for (std::map<std::string, std::string>::iterator it = cfg_map.begin(); it != cfg_map.end(); ++it) {
            fprintf(fp, "%s = %s\n", it->first.c_str(), it->second.c_str());
        }
    }
    const sds get_errmsg() { return errmsg; }
private:
    std::map<std::string, std::string> cfg_map;
    sds errmsg;
};

#ifdef CFG_HELPER_TEST

int cfg_helper_test();

#endif      // CFG_HELPER_TEST

}

#endif      // _CFG_HELPER_
