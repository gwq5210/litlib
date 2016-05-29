/*************************************************************************
	> File Name: datetime.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月17日 星期二 00时25分01秒
 ************************************************************************/

#include <unistd.h>

#include "datetime.h"
#include "jemalloc/jemalloc.h"

namespace util {

int datetime::format_time(sds &s, const char *fmt)
{
    char static_buf[1024];
    char *buf = static_buf;
    int buflen = sizeof(static_buf);
    while (1) {
        buf[buflen - 2] = '\0';
        strftime(buf, buflen, fmt, &d);
        if (buf[buflen - 2] == '\0') {
            break;
        } else {
            if (buf != static_buf) {
                jefree(buf);
            }
            buflen *= 2;
            buf = (char *)jemalloc(buflen);
            if (buf == NULL) {
                set_errmsg(errmsg, "jemalloc error!");
                return ERR_ALLOC;
            }
        }
    }
    sdscpy(s, buf);
    if (s == NULL) {
        set_errmsg(errmsg, "sdscpy error!");
        return ERR_LIBCALL;
    }
    return 0;
}

#ifdef DATETIME_TEST

int datetime_test()
{
    datetime date;
    sds s = sdsnewlen(1024);
    date.to_string(s);
    printf("%s\n", s);
    sdsfree(s);

    printf("%d\n", date.get_year());
    printf("%d\n", date.get_mon());
    printf("%d\n", date.get_mday());
    printf("%d\n", date.get_hour());
    printf("%d\n", date.get_min());
    printf("%d\n", date.get_sec());
    printf("%d\n", date.get_week());
    printf("%d\n", date.get_usec());
    printf("%d\n", date.get_seconds());

    sleep(1);

    date.report("test");
    return 0;
}

#endif      // DATETIME_TEST

}
