/*************************************************************************
	> File Name: mysql_helper.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月25日 星期一 23时19分42秒
 ************************************************************************/

#ifndef _MYSQL_HELPER_H
#define _MYSQL_HELPER_H

#include "mysql/mysql.h"

#include "sds.h"
#include "ret_status.h"

namespace util {

class mysql_helper {
public:
    mysql_helper(): conn(NULL), result(NULL), errmsg(NULL), host(NULL), user(NULL), passwd(NULL), db_name(NULL), port(0),
        unix_socket(NULL), client_flag(0), charset(NULL), connect_timeout_s(-1), read_timeout_s(-1), write_timeout_s(-1), sql_buf(NULL) { init(); }
    ~mysql_helper()
    {
        clear();
    }
    int connect(const char *db_host, const char *db_user, const char *db_passwd, const char *db,
        int db_port = 0, int db_connect_timeout_s = -1, int db_read_timeout_s = -1, int db_write_timeout_s = -1,
        const char *db_unix_socket = NULL, unsigned long db_client_flag = 0, const char *db_charset = "utf8");
    int reconnect(const char *db_host, const char *db_user, const char *db_passwd, const char *db,
        int db_port = 0, int db_connect_timeout_s = -1, int db_read_timeout_s = -1, int db_write_timeout_s = -1,
        const char *db_unix_socket = NULL, unsigned long db_client_flag = 0, const char *db_charset = "utf8")
    {
        clear();
        init();
        return connect(db_host, db_user, db_passwd, db, db_port, db_connect_timeout_s, db_read_timeout_s,
            db_write_timeout_s, db_unix_socket, db_client_flag, db_charset);
    }
    int ping()
    {
        if (conn == NULL || mysql_ping(conn)) {
            return reconnect(host, user, passwd, db_name, port, connect_timeout_s, read_timeout_s,
                write_timeout_s, unix_socket, client_flag, charset);
        }
        return 0;
    }
    int init()
    {
        errmsg = sdsnewlen(1024);
        if (errmsg == NULL) {
            return ERR_LIBCALL;
        }
        sql_buf = sdsnewlen(1024); 
        if (sql_buf == NULL) {
            return ERR_LIBCALL;
        }
        return 0;
    }
    int clear()
    {
        if (conn) {
            mysql_close(conn);
            conn = NULL;
        }
        free_result();
        sdsfree(errmsg);
        sdsfree(host);
        sdsfree(user);
        sdsfree(passwd);
        sdsfree(db_name);
        sdsfree(unix_socket);
        sdsfree(charset);
        sdsfree(sql_buf);
        mysql_library_end();
        return 0;
    }
    const sds get_errmsg() { return errmsg; }
    int query(const char *fmt, ...);
    int update(const char *fmt, ...);
    const MYSQL_ROW fetch_row()
    {
        if (result) {
            return mysql_fetch_row(result);
        }
        return NULL;
    }
    int free_result()
    {
        if (result) {
            mysql_free_result(result); 
            result = NULL;
        }
        return 0;
    }
private:
    MYSQL *conn;
    MYSQL_RES *result;
    sds errmsg;
    sds host;
    sds user;
    sds passwd;
    sds db_name;
    int port;
    sds unix_socket;
    unsigned long client_flag;
    sds charset;
    int connect_timeout_s;
    int read_timeout_s;
    int write_timeout_s;
    sds sql_buf;
};

#ifdef MYSQL_HELPER_TEST

int mysql_helper_test();

#endif      // MYSQL_HELPER_TEST

}

#endif      // _MYSQL_HELPER_H
