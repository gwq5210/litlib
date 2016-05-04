/*************************************************************************
	> File Name: mysql_helper.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年04月25日 星期一 23时19分26秒
 ************************************************************************/

#include "mysql_helper.h"
#include "common_tool.h"

namespace util {

int mysql_helper::connect(const char *db_host, const char *db_user, const char *db_passwd, const char *db,
    int db_port/* = 0*/, int db_connect_timeout_s/* = -1*/, int db_read_timeout_s/* = -1*/, int db_write_timeout_s/* = -1*/,
    const char *db_unix_socket/* = NULL*/, unsigned long db_client_flag/* = 0*/, const char *db_charset/* = "utf8"*/)
{
    conn = mysql_init(NULL);
    if (conn == NULL) {
        set_errmsg(errmsg, "mysql_init call error!");
        return ERR_LIBCALL;
    }
    host = sdsnew(db_host);
    user = sdsnew(db_user);
    passwd = sdsnew(db_passwd);
    db_name = sdsnew(db);
    port = db_port;
    if (db_unix_socket) {
        unix_socket = sdsnew(db_unix_socket);
        if (unix_socket == NULL) {
            set_errmsg(errmsg, "sdsnew call error!");
            return ERR_LIBCALL;
        }
    }
    client_flag = db_client_flag;
    charset = sdsnew(db_charset);

    if (host == NULL || user == NULL || passwd == NULL || db_name == NULL || charset == NULL || sql_buf == NULL) {
        set_errmsg(errmsg, "sdsnew call error!");
        return ERR_LIBCALL;
    }

    if (db_read_timeout_s > 0) {
        read_timeout_s = db_read_timeout_s;
        mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, (const char *)&read_timeout_s);
    }
    if (db_write_timeout_s > 0) {
        write_timeout_s = db_write_timeout_s;
        mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, (const char *)&write_timeout_s);
    }
    if (db_connect_timeout_s > 0) {
        connect_timeout_s = db_connect_timeout_s;
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&connect_timeout_s);
    }
    mysql_options(conn, MYSQL_OPT_RECONNECT, "1");

    if (mysql_real_connect(conn, host, user, passwd, db_name, port, unix_socket, client_flag) == NULL) {
        set_errmsg(errmsg, "mysql_real_connect call error! error code[%d], %s", mysql_errno(conn), mysql_error(conn));
        return ERR_LIBCALL;
    }

    if (mysql_set_character_set(conn, charset) != 0) {
        set_errmsg(errmsg, "mysql_set_character_set call error! error code[%d], %s", mysql_errno(conn), mysql_error(conn));
        return ERR_LIBCALL;
    }

    return 0;
}

int mysql_helper::query(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsdsprintf(sql_buf, fmt, ap);
    va_end(ap);
    if (ret < 0) {
        set_errmsg(errmsg, "vsdsprintf call error!");
        return ERR_LIBCALL;
    }

    ping();
    if (mysql_real_query(conn, sql_buf, sdslen(sql_buf)) != 0) {
        set_errmsg(errmsg, "mysql_real_connect call error! error code[%d], %s", mysql_errno(conn), mysql_error(conn));
        return ERR_LIBCALL;
    }

    free_result();
    result = mysql_store_result(conn);
    if (result == NULL) {
        set_errmsg(errmsg, "mysql_store_result call error! error code[%d], %s", mysql_errno(conn), mysql_error(conn));
        return ERR_LIBCALL;
    }
    return mysql_num_rows(result);
}

int mysql_helper::update(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsdsprintf(sql_buf, fmt, ap);
    va_end(ap);
    if (ret < 0) {
        set_errmsg(errmsg, "vsdsprintf call error!");
        return ERR_LIBCALL;
    }

    ping();
    if (mysql_real_query(conn, sql_buf, sdslen(sql_buf)) != 0) {
        set_errmsg(errmsg, "mysql_real_connect call error! error code[%d], %s", mysql_errno(conn), mysql_error(conn));
        return ERR_LIBCALL;
    }

    return mysql_affected_rows(conn);
}

#ifdef MYSQL_HELPER_TEST

int mysql_helper_test()
{
    const char *id = "1234567";
    int num_rows = -1;
    {
        mysql_helper sql_handle;
        if (sql_handle.connect("127.0.0.1", "root", "1234", "test") < 0) {
            printf("connect error! %s\n", sql_handle.get_errmsg());
            return -1;
        }

        if ((num_rows = sql_handle.query("select id from students where id='%s'", id)) < 0) {
            printf("query error! %s\n", sql_handle.get_errmsg());
            return -1;
        } else {
            printf("query success! result num rows[%d]\n", num_rows);
        }

        if (num_rows == 0) {
            printf("students %s is not exist! insert it!\n", id);
            int affected_rows = -1;
            if ((affected_rows = sql_handle.update("insert into students values('%s', '你好', '男', '18838096261', 'gwq5210@qq.com')", id)) < 0) {
                printf("insert error! %s\n", sql_handle.get_errmsg());
                return -1;
            } else {
                printf("insert success! affected_rows[%d]!\n", affected_rows);
            }
        } else {
            printf("students %s is exist! do nothing!\n", "1234567");
        }
    }

    mysql_helper sql_handle_new;
    if (sql_handle_new.connect("127.0.0.1", "root", "1234", "test") < 0) {
        printf("connect error! %s\n", sql_handle_new.get_errmsg());
        return -1;
    }
    if ((num_rows = sql_handle_new.query("select * from students")) < 0) {
        printf("query error! %s\n", sql_handle_new.get_errmsg());
        return -1;
    } else {
        printf("query success! result num rows[%d]\n", num_rows);
    }
    MYSQL_ROW row;
    while ((row = sql_handle_new.fetch_row()) != NULL) {
        printf("%-15s %-15s %-15s %-15s %-20s\n", row[0], row[1], row[2], row[3], row[4]);
    }
	return 0;
}

#endif //       MYSQL_HELPER_TEST

}
