/*************************************************************************
	> File Name: datetime.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月17日 星期二 00时25分15秒
 ************************************************************************/

#ifndef _DATETIME_H
#define _DATETIME_H

#include "sds.h"
#include "ret_status.h"
#include "common_tool.h"

#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

namespace util {

class datetime {
public:
    datetime(): errmsg(NULL), year(-1), month(-1), day(-1), week(-1), hour(-1), minute(-1), second(-1)
    {
        errmsg = sdsnewlen(1024);
        memset(&tv, 0, sizeof(struct timeval));
        memset(&d, 0, sizeof(struct tm));
        set_time();
    }
    datetime(time_t t): errmsg(NULL), year(-1), month(-1), day(-1), week(-1), hour(-1), minute(-1), second(-1)
    {
        errmsg = sdsnewlen(1024);
        memset(&tv, 0, sizeof(struct timeval));
        memset(&d, 0, sizeof(struct tm));
        set_time(t);
    }
    datetime(const struct timeval *ptv): errmsg(NULL), year(-1), month(-1), day(-1), week(-1), hour(-1), minute(-1), second(-1)
    {
        errmsg = sdsnewlen(1024);
        memset(&tv, 0, sizeof(struct timeval));
        memset(&d, 0, sizeof(struct tm));
        set_time(ptv);
    }
    ~datetime()
    {
        sdsfree(errmsg);
    }
    int set_time()
    {
        if (gettimeofday(&tv, NULL) < 0) {
            set_errmsg(errmsg, "gettimeofday error! errno[%d], %s", errno, strerror(errno));
            return ERR_SYSCALL;
        }
        struct tm *date = localtime(&tv.tv_sec);
        if (date == NULL) {
            set_errmsg(errmsg, "localtime error! date is NULL!");
            return ERR_SYSCALL;
        }
        return set_tm(date);
    }
    int set_time(time_t t)
    {
        tv.tv_sec = t;
        tv.tv_usec = 0;
        struct tm *date = localtime(&tv.tv_sec);
        if (date == NULL) {
            set_errmsg(errmsg, "localtime error! date is NULL!");
            return ERR_SYSCALL;
        }
        return set_tm(date);
    }
    int set_time(const struct timeval *ptv)
    {
        tv = *ptv;
        struct tm *date = localtime(&tv.tv_sec);
        if (date == NULL) {
            set_errmsg(errmsg, "localtime error! date is NULL!");
            return ERR_SYSCALL;
        }
        return set_tm(date);
    }
    int get_year() { return year; }
    int get_mon() { return month; }
    int get_mday() { return day; }
    int get_week() { return week; }
    int get_hour() { return hour; }
    int get_min() { return minute; }
    int get_sec() { return second; }
    int get_usec() { return tv.tv_usec; }
    int get_seconds() { return tv.tv_sec; }
    const sds get_errmsg() { return errmsg; }
    int to_string(sds &s) { return format_time(s, "%Y-%m-%d %H:%M:%S"); }
    int format_time(sds &s, const char *fmt);
    struct timeval timevaldiff()
    {
        struct timeval ret;
        struct timeval end_tv;
        ret.tv_sec = 0;
        ret.tv_usec = 0;
        if (gettimeofday(&end_tv, NULL) < 0) {
            set_errmsg(errmsg, "gettimeofday error! errno[%d], %s", errno, strerror(errno));
            return ret;
        }

        long long seconds = end_tv.tv_sec - tv.tv_sec;
        long long useconds = end_tv.tv_usec - tv.tv_usec;
        if (useconds < 0) {
            useconds += 1000000;
            seconds -= 1;
        }
        ret.tv_sec = seconds;
        ret.tv_usec = useconds;
        return ret;
    }
    time_t timediff()
    {
        struct timeval diff = timevaldiff();
        return diff.tv_sec;
    }
    int report(const char *name, FILE *fp = stdout)
    {
        struct timeval diff = timevaldiff();
        long long seconds = diff.tv_sec;
        long long useconds = diff.tv_usec;
        fprintf(fp, "%s end! cost %lld seconds and %lld useconds!\n", name, seconds, useconds);
        return 0;
    }
private:
    int set_tm(const tm *date)
    {
        if (date == NULL) {
            set_errmsg(errmsg, "datetime is NULL!");
            return ERR_PARAM;
        }
        d = *date;
        year = d.tm_year + 1900;
        month = d.tm_mon + 1;
        day = d.tm_mday;
        week = d.tm_wday ? d.tm_wday : d.tm_wday + 7;
        hour = d.tm_hour;
        minute = d.tm_min;
        second = d.tm_sec;
        return 0;
    }
private:
    struct timeval tv;
    struct tm d;
    sds errmsg;
    int year;
    int month;
    int day;
    int week;
    int hour;
    int minute;
    int second;
};

#ifdef DATETIME_TEST

int datetime_test();

#endif      // DATETIME_TEST

}

#endif      // _DATETIME_H
