/*************************************************************************
	> File Name: cfg_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月25日 星期一 23时22分19秒
 ************************************************************************/

#include <stdlib.h>
#include <unistd.h>

#include "cfg_helper.h"
#include "common_tool.h"

namespace util {

/*
 * file format:
 * # comments
 * cfg_name = cfg_val   # comments
 */
int cfg_helper::read_cfg(const char *cfg_file_name)
{
    if (access(cfg_file_name, F_OK) < 0) {
        set_errmsg(errmsg, "access call error! cfg file[%s] not exist!", cfg_file_name);
        return ERR_LIBCALL;
    }

    FILE *fp = fopen(cfg_file_name, "r");
    sds s = NULL;
    while ((s = read_line(fp)) != NULL) {
        sdstrim(s);
        if (s[0] == '#') {
            sdsfree(s);
            continue;
        }
        int count = 0;
        sds *res = sdssplit(s, "=", count, SPLIT_FIRST);
        if (count != 2) {
            sdssplitfree(res, count);
            sdsfree(s);
            continue;
        }
        sdstrim(res[0]);
        std::string cfg_name(res[0]);

        int value_count = 0;
        sds *value_res = sdssplit(res[1], "#", value_count, SPLIT_FIRST);
        if (res == NULL) {
            sdssplitfree(res, count);
            sdsfree(s);
            continue;
        }
        sdstrim(value_res[0]);
        std::string cfg_value(value_res[0]);
        cfg_map[cfg_name] = cfg_value;

        sdssplitfree(res, count);
        sdssplitfree(value_res, value_count);
        sdsfree(s);
    }
    fclose(fp);

    return 0;
}

int cfg_helper::get_int(const char *cfg_name, int default_val/* = -1*/)
{
    if (!is_have(cfg_name)) {
        return default_val;
    }

    std::string name(cfg_name);
    return atoi(cfg_map[name].c_str());
}

sds cfg_helper::get_str(const char *cfg_name, const char *default_val/* = "invalid"*/)
{
    if (!is_have(cfg_name)) {
        return sdsnew(default_val);
    }

    std::string name(cfg_name);
    return sdsnew(cfg_map[name].c_str());
}

double cfg_helper::get_dbl(const char *cfg_name, double default_val/* = -1.0*/)
{
    if (!is_have(cfg_name)) {
        return default_val;
    }

    std::string name(cfg_name);
    return atof(cfg_map[name].c_str());
}

bool cfg_helper::is_have(const char *cfg_name)
{
    std::string name(cfg_name);
    if (!cfg_map.count(name)) {
        return false;
    }
    return true;
}

#ifdef CFG_HELPER_TEST

int cfg_helper_test()
{
    cfg_helper cfg;
    const char *cfg_file_name = "test.cfg";
    if (cfg.read_cfg(cfg_file_name) < 0) {
        printf("read_cfg %s error! %s\n", cfg_file_name, cfg.get_errmsg());
        return -1;
    }
    cfg.print();

    double d = cfg.get_dbl("haha");
    printf("%.2f\n", d);
    int age = cfg.get_int("age");
    printf("%d\n", age);
    sds name = cfg.get_str("name");
    printf("%s\n", name);
    sdsfree(name);
    int num = cfg.get_int("num");
    printf("%d\n", num);

    return 0;
}

#endif      // CFG_HELPER_TEST

}
