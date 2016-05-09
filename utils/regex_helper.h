/*************************************************************************
	> File Name: regex_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月05日 星期四 11时34分36秒
 ************************************************************************/

#ifndef _REGEX_HELPER_H
#define _REGEX_HELPER_H

#include <sys/types.h>
#include <regex.h>

#include "jemalloc/jemalloc.h"
#include "sds.h"
#include "error.h"

namespace util {

#define REPLACE_ALL 0
#define REPLACE_FIRST 1
#define REPLACE_LAST 2

class regex_helper {
public:
    regex_helper(): reg(NULL), result(NULL), match_result(NULL), count(0), errmsg(NULL), cflags(0), eflags(0) { init(); }
    regex_helper(const char *reg_str, int reg_cflags): reg(NULL), result(NULL), match_result(NULL), count(0), errmsg(NULL), cflags(0), eflags(0)
    {
        init();
        comp(reg_str, reg_cflags);
    }
    ~regex_helper()
    {
        clear();
    }
    static sds replace(const char *str, const char *replace_str, regex_helper &reg, int replace_type = REPLACE_ALL);
    int comp(const char * reg_str, int reg_cflags = REG_EXTENDED);
    int recomp(const char *reg_str, int reg_cflags = REG_EXTENDED)
    {
        clear();
        init();
        return comp(reg_str, cflags);
    }
    int match(const char *match_str, int cnt = 0, int reg_eflags = 0);
    sds replace(const char *str, const char *replace_str, int replace_type = REPLACE_ALL)
    {
        return replace(str, replace_str, *this, replace_type);
    }
    const sds group(int cnt)
    {
        if (cnt < 0 || cnt >= count)
        {
            return NULL;
        }
        return result[cnt];
    }
    const regmatch_t *group_pos(int cnt)
    {
        if (cnt < 0 || cnt >= count)
        {
            return NULL;
        }
        return &match_result[cnt];
    }
    int result_count() { return count; }
    sds get_regerror(int ret_code);
    const sds get_errmsg() { return errmsg; }
    int free_result()
    {
        if (result == NULL && match_result == NULL) {
            return 0;
        }

        if (result != NULL) {
            for (int i = 0; i < count; ++i) {
                sdsfree(result[i]);
            }
            jefree(result);
            result = NULL;
        }
        if (match_result != NULL) {
            jefree(match_result);
            match_result = NULL;
        }
        count = 0;
        return 0;
    }
    int init()
    {
        reg = (regex_t *)jemalloc(sizeof(regex_t));
        if (reg == NULL) {
            return ERR_ALLOC;
        }
        errmsg = sdsnewlen(1024); 
        if (errmsg == NULL) {
            return ERR_LIBCALL;
        }
        count = 0;
        result = NULL;
        return 0;
    }
    int clear()
    {
        if (reg != NULL) {
            regfree(reg);
            jefree(reg);
            reg = NULL;
        }
        free_result();
        sdsfree(errmsg);
        count = 0;
        return 0;
    }
private:
    regex_t *reg;
    sds *result;
    regmatch_t *match_result;
    int count;
    sds errmsg;
    int cflags;
    int eflags;
};

#ifdef REGEX_HELPER_TEST

int regex_helper_test();

#endif      // REGEX_HELPER_TEST

}

#endif      // _REGEX_HELPER_H
