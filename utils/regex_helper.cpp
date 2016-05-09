/*************************************************************************
	> File Name: regex_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月05日 星期四 11时34分15秒
 ************************************************************************/

#include <string.h>

#include "regex_helper.h"
#include "common_tool.h"
#include "test_helper.h"

namespace util {

sds regex_helper::replace(const char *str, const char *replace_str, regex_helper &reg, int replace_type/* = REPLACE_ALL*/)
{
    if (str == NULL) {
        return NULL;
    }
    if (replace_str == NULL || *replace_str == '\0') {
        return sdsnew(str);
    }

    sds buf = sdsnewlen(strlen(str));
    if (buf == NULL) {
        return NULL;
    }
    regmatch_t match_pos;
    int start = 0;
    while (1) {
        int ret = reg.match(str + start, 1);
        if (ret == ERR_REG_NOMATCH) {
            break;
        } else if (ret != 0) {
            sdsfree(buf);
            return NULL;
        }
        match_pos = *reg.group_pos(0);
        if (replace_type != REPLACE_LAST) {
            sdscatlen(buf, str + start, start + match_pos.rm_so - start);
            if (buf == NULL) {
                return NULL;
            }
            sdscat(buf, replace_str);
            if (buf == NULL) {
                return NULL;
            }
        }
        start += match_pos.rm_eo;
        if (replace_type == REPLACE_FIRST) {
            break;
        }
    }
    if (replace_type == REPLACE_LAST && start != 0) {
        start -= match_pos.rm_eo;
        sdscatlen(buf, str, start + match_pos.rm_so);
        if (buf == NULL) {
            return NULL;
        }
        sdscat(buf, replace_str);
        if (buf == NULL) {
            return NULL;
        }
        start += match_pos.rm_eo;
    }
    sdscat(buf, str + start);
    if (buf == NULL) {
        return NULL;
    }
    return buf;
}

int regex_helper::comp(const char *reg_str, int reg_cflags/* = REG_EXTENDED*/)
{
    cflags = reg_cflags;
    int ret_code = regcomp(reg, reg_str, cflags);
    if (ret_code != 0) {
        sds s = get_regerror(ret_code);
        set_errmsg(errmsg, "regcomp error! error code[%d], %s", ret_code, s);
        sdsfree(s);
        return ERR_LIBCALL;
    }
    return 0;
}

int regex_helper::match(const char *match_str, int cnt/* = 0*/, int reg_eflags/* = 0*/) {
    free_result();

    eflags = reg_eflags;
    int guess_count = 10;
    if (cnt > 0) {
        guess_count = cnt;
    }
    result = (sds *)jemalloc(sizeof(sds *) * guess_count);
    match_result = (regmatch_t *)jemalloc(sizeof(regmatch_t) * guess_count);
    if (result == NULL || match_result == NULL) {
        free_result();
        return ERR_ALLOC;
    }
    memset(result, 0, sizeof(sds *) * guess_count);
    memset(match_result, -1, sizeof(regmatch_t) * guess_count);
    count = 0;
    while (1) {
       int ret_code = regexec(reg, match_str, guess_count, match_result, eflags);
       if (ret_code != 0) {
           sds s = get_regerror(ret_code);
           set_errmsg(errmsg, "regexec error! error code[%d], %s", ret_code, s);
           sdsfree(s);

           free_result();
           if (ret_code == REG_NOMATCH) {
               return ERR_REG_NOMATCH;
           } else {
               return ERR_LIBCALL;
           }
       }
       if (cnt > 0) {
           break;
       }
       if (match_result[guess_count - 1].rm_so == -1) {
           break;
       } else {
           free_result();
           guess_count *= 2;
           result = (sds *)jemalloc(sizeof(sds *) * guess_count);
           match_result = (regmatch_t *)jemalloc(sizeof(regmatch_t) * guess_count);
           if (result == NULL || match_result == NULL) {
               free_result();
               return ERR_ALLOC;
           }
           memset(result, 0, sizeof(sds *) * guess_count);
           memset(match_result, -1, sizeof(regmatch_t) * guess_count);
           count = 0;
       }
    }

    for (int i = 0; i < guess_count && match_result[i].rm_so != -1; ++i, ++count) {
        result[i] = sdsnewlen(match_str + match_result[i].rm_so, match_result[i].rm_eo - match_result[i].rm_so);
        if (result[i] == NULL) {
            free_result();
            return ERR_LIBCALL;
        }
    }

    return 0;
}

sds regex_helper::get_regerror(int ret_code)
{
    char static_buf[1024];
    int buflen = sizeof(static_buf);
    char *buf = static_buf;
    while (1) {
        buf[buflen - 2] = '\0';
        regerror(ret_code, reg, buf, buflen);
        if (buf[buflen - 2] == '\0') {
            break;
        } else {
            buflen *= 2;
            if (buf != static_buf) {
                jefree(buf);
            }
            buf = (char *)jemalloc(buflen);
            if (buf == NULL) {
                return NULL;
            }
        }
    }
    sds s = sdsnew(buf);
    if (buf != static_buf) {
        jefree(buf);
    }
    return s;
}

#ifdef REGEX_HELPER_TEST

int regex_helper_test()
{
    const char *reg_str = "q(q)";
    regex_helper reg;
    //int ret = reg.comp("^q(q)$", REG_NOSUB | REG_EXTENDED);
    int ret = reg.comp(reg_str, REG_EXTENDED);
    if (ret < 0) {
        printf("compile regex[%s] error! %s\n", reg_str, reg.get_errmsg());
        return -1;
    }

    //ret = reg.match("gwq5210@qq.com");
    const char *match_str = "qqq";
    ret = reg.match(match_str);
    if (ret == ERR_REG_NOMATCH) {
        printf("no match! %s\n", reg.get_errmsg());
    } else if (ret != 0) {
        printf("match error! %s\n", reg.get_errmsg());
    } else {
        printf("match!\n");
        int cnt = reg.result_count();
        for (int i = 0; i < cnt; ++i) {
            const sds s = reg.group(i);
            const regmatch_t *match_pos = reg.group_pos(i);
            printf("%d %d %s\n", match_pos->rm_so, match_pos->rm_eo, s);
        }
    }

    const char *str = "qqqq";
    const char *replace_str = "xy";
    sds name = sdsnewlen(1024);
    sdsprintf(name, "replace(\"%s\", \"%s\", \"%s\")", str, replace_str, reg_str);
    sds s = regex_helper::replace(str, replace_str, reg);
    if (s == NULL) {
        printf("replace error!");
    } else {
        printf("%s\n", s);
    }
    test_cond(name, strcmp(s, "xyxy") == 0);
    sdsfree(s);

    s = regex_helper::replace(str, replace_str, reg, REPLACE_FIRST);
    if (s == NULL) {
        printf("replace error!");
    } else {
        printf("%s\n", s);
    }
    test_cond(name, strcmp(s, "xyqq") == 0);
    sdsfree(s);

    s = regex_helper::replace(str, replace_str, reg, REPLACE_LAST);
    if (s == NULL) {
        printf("replace error!");
    } else {
        printf("%s\n", s);
    }
    test_cond(name, strcmp(s, "qqxy") == 0);
    sdsfree(s);

    sdsfree(name);
    return 0;
}

#endif      // REGEX_HELPER_TEST

}
