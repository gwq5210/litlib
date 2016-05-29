/*************************************************************************
	> File Name: logger.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月17日 星期二 01时56分26秒
 ************************************************************************/

#ifndef _LOGGER_H
#define _LOGGER_H

#include "datetime.h"
#include "ret_status.h"

#include <stdarg.h>

#define DEFAULT_MAX_FILE_NUM 10
#define DEFAULT_ONE_FILE_SIZE (512 * 1024 * 1024)

namespace util {

enum log_level {
    LOG_OFF = -1,
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARN = 2,
    LOG_ERROR = 3,
    LOG_FATAL = 4
};

class logger {
public:
    logger(const char *log_basename): log_level(LOG_DEBUG), write_num(0), max_write_num(-1), max_file_num(DEFAULT_MAX_FILE_NUM),
        one_file_size(DEFAULT_ONE_FILE_SIZE), curr_name(NULL), basename(NULL), errmsg(NULL) { init(log_basename); }
    ~logger() { clear(); }
    int debug(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int ret = vwrite_log(LOG_DEBUG, fmt, ap);
        va_end(ap);
        return ret;
    } 
    int info(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int ret = vwrite_log(LOG_INFO, fmt, ap);
        va_end(ap);
        return ret;
    } 
    int wran(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int ret = vwrite_log(LOG_WARN, fmt, ap);
        va_end(ap);
        return ret;
    } 
    int error(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int ret = vwrite_log(LOG_ERROR, fmt, ap);
        va_end(ap);
        return ret;
    } 
    int fatal(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int ret = vwrite_log(LOG_FATAL, fmt, ap);
        va_end(ap);
        return ret;
    } 
    int write_log(int level, const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int ret = vwrite_log(level, fmt, ap);
        va_end(ap);
        return ret;
    } 
    int vwrite_log(int level, const char *fmt, va_list ap);
    int check_load();
    int check_file();
    void set_load(int new_max_write_num) { max_write_num = new_max_write_num; }
    int get_load(int new_max_write_num) { return max_write_num; }
    int set_file_size(int size)
    {
        if (size <= 0) {
            set_errmsg(errmsg, "param error! file size less equal zero!");
            return ERR_PARAM;
        }
        one_file_size = size;
        return 0;
    }
    int get_file_size() { return one_file_size; }
    int set_file_num(int file_num)
    {
        if (file_num <= 0) {
            set_errmsg(errmsg, "param error! file num less equal zero!");
            return ERR_PARAM;
        }
        max_file_num = file_num;
        return 0;
    }
    int get_file_num() { return max_file_num; }
    int set_log_level(int level)
    {
        if (level < LOG_OFF || level > LOG_FATAL) {
            set_errmsg(errmsg, "param error! level[%d]!", level);
            return ERR_PARAM;
        }
        log_level = level;
    }
    int get_log_level() { return log_level; }
    const sds get_errmsg() { return errmsg; }
private:
    int init(const char *log_basename)
    {
        errmsg = sdsnewlen(1024);
        if (errmsg == NULL) {
            return ERR_LIBCALL;
        }
        basename = sdsnew(log_basename);
        if (basename == NULL) {
            set_errmsg(errmsg, "sdsnew error! basename is NULL!");
            return ERR_LIBCALL;
        }
        curr_name = sdsnewlen(1024);
        if (curr_name == NULL) {
            set_errmsg(errmsg, "sdsnew error! curr_name is NULL!");
            return ERR_LIBCALL;
        }
        sdsprintf(curr_name, "%s.log", basename);
        return 0;
    }
    int clear()
    {
        sdsfree(errmsg);
        sdsfree(curr_name);
        sdsfree(basename);
        return 0;
    }
private:
    int log_level;
    datetime last_log_time;
    int write_num;
    int max_write_num;
    int max_file_num;
    int one_file_size;
    sds curr_name;
    sds basename;
    sds errmsg;
};

#ifdef LOGGER_TEST

int logger_test();

#endif      // LOGGER_TEST

}

#endif      // _LOGGER_H
