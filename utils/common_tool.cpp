/*************************************************************************
	> File Name: common_tool.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月03日 星期二 14时04分40秒
 ************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "common_tool.h"
#include "jemalloc/jemalloc.h"

#include "ret_status.h"
#include "epoll_helper.h"

namespace util {

sds load_file(const char *file_name)
{
    struct stat file_info;
    if (access(file_name, F_OK) < 0) {
        return NULL;
    }
    if (stat(file_name, &file_info) < 0) {
        return NULL;
    }
    size_t file_size = file_info.st_size;
    FILE *fp = fopen(file_name, "rb");
    if (fp == NULL) {
        return NULL;
    }
    char *buf = (char *)jemalloc(file_size);
    if (buf == NULL) {
        return NULL;
    }
    size_t read_num = fread(buf, file_size, 1, fp);
    if (read_num != 1) {
        jefree(buf);
        return NULL;
    }
    sds s = sdsnewlen(buf, file_size);
    if (s == NULL) {
        jefree(buf);
        return NULL;
    }
    jefree(buf);
    fclose(fp);
    return s;
}

sds load_file(FILE *fp)
{
    if (fp == NULL) {
        return NULL;
    }
    char buf[1024];
    int buflen = sizeof(buf);
    sds s = sdsnewlen(1024);
    if (s == NULL) {
        return NULL;
    }
    do {
        size_t read_size = fread(buf, 1, buflen, fp);
        if (read_size <= 0) {
            if (feof(fp) != 0) {
                break;
            } else if (ferror(fp) != 0) {
                sdsfree(s);
                return NULL;
            }
        } else {
            s = sdscatlen(s, buf, read_size);
        }
    } while (1);

    return s;
}

int save_file(const sds s, const char *file_name)
{
    FILE *fp = fopen(file_name, "wb");
    if (fp == NULL) {
        return ERR_SYSCALL;
    }

    int file_size = sdslen(s);
    int write_num = fwrite(s, file_size, 1, fp);
    if (write_num != 1) {
        return ERR_SYSCALL;
    }
    fclose(fp);
    return 0;
}

sds read_line(FILE *fp)
{
    if (fp == NULL) {
        return NULL;
    }

    long cur_pos = ftell(fp);
    if (cur_pos < 0) {
        return NULL;
    }

    char static_buf[1024];
    char *buf = static_buf;
    int buflen = sizeof(static_buf);

    buf[buflen - 2] = '\0';
    while (1) {
        if (fgets(buf, buflen, fp) == NULL) {
            if (buf != static_buf) {
                jefree(buf);
            }
            return NULL;
        }
        if (buf[buflen - 2] == '\0') {
            break;
        } else {
            if (fseek(fp, cur_pos, SEEK_SET) < 0) {
                return NULL;
            }
            if (buf != static_buf) {
                jefree(buf);
            }
            buflen *= 2;
            buf = (char *)jemalloc(buflen);
            if (buf == NULL) {
                return NULL;
            }
            buf[buflen - 2] = '\0';
        }
    }

    sds s = sdsnew(buf);
    if (buf != static_buf) {
        jefree(buf);
    }
    sdstrim(s, "\n");
    return s;
}

int socket_read(int socket_fd, sds &read_buf, sds &errmsg, int timeout_ms/* = -1*/, int should_len/* = -1*/)
{
    if (read_buf == NULL) {
        set_errmsg(errmsg, "param error! read_buf is NULL");
        return ERR_PARAM;
    }

    if (socket_fd < 0) {
        set_errmsg(errmsg, "no connect! socket_fd[%d]", socket_fd);
        return ERR_NOINIT;
    }

    epoll_helper epoll;
    if (epoll.create(64) < 0) {
        set_errmsg(errmsg, "epoll create error! %s", epoll.get_errmsg());
        return ERR_LIBCALL;
    }
    epoll.add(socket_fd, EPOLLIN);
    int ret = epoll.wait(timeout_ms);
    if (ret < 0) {
        set_errmsg(errmsg, "wait error! %s", epoll.get_errmsg());
        return ERR_LIBCALL;
    } else if (ret == 0) {
        set_errmsg(errmsg, "read timeout! no fd reday!");
        return ERR_TIMEOUT;
    }

    int flag = 0;
    if (should_len <= 0) {
        flag = fcntl(socket_fd, F_GETFL) | O_NONBLOCK;
        if (flag < 0) {
            set_errmsg(errmsg, "fcntl get fd[%d] flag error! errno[%d], %s", socket_fd, errno, strerror(errno));
            return ERR_SYSCALL;
        }
        if (fcntl(socket_fd, F_SETFL, flag) < 0) {
            set_errmsg(errmsg, "fcntl set fd[%d] flag[%d] error! errno[%d], %s", socket_fd, flag, errno, strerror(errno));
            return ERR_SYSCALL;
        }
    }

    int len = should_len;
    int read_len = 0;
    char buf[1024];
    int buflen = sizeof(buf);
    sdsclear(read_buf);
    while (1) {
        if (should_len > 0) {
            if (len <= 0) {
                break;
            }
            if (len < buflen) {
                buflen = len;
            }
        }
        read_len = read(socket_fd, buf, buflen);
        if (read_len < 0) {
            if (errno == EAGAIN) {
                break;
            }
            set_errmsg(errmsg, "read error! errno[%d], %s", errno, strerror(errno));
            return ERR_LIBCALL;
        } else if (read_len == 0) {
            break;
        }
        sdscatlen(read_buf, buf, read_len);
        len -= read_len;
    }

    if (should_len <= 0) {
        flag &= ~O_NONBLOCK;
        if (fcntl(socket_fd, F_SETFL, flag) < 0) {
            set_errmsg(errmsg, "fcntl set fd[%d] flag[%d] error! errno[%d], %s", socket_fd, flag, errno, strerror(errno));
            return ERR_SYSCALL;
        }
    }

    return 0;
}

int socket_write(int socket_fd, const char *write_buf, int buflen, sds &errmsg)
{
    if (write_buf == NULL || buflen <= 0) {
        set_errmsg(errmsg, "param error! write_buf is NULL or buflen less then zero!");
        return ERR_PARAM;
    }

    if (socket_fd < 0) {
        set_errmsg(errmsg, "no connect! socket_fd[%d]", socket_fd);
        return ERR_NOINIT;
    }

    int done = 0;
    while (done < buflen) {
        int ret = write(socket_fd, write_buf + done, buflen - done);
        if (ret < 0) {
            set_errmsg(errmsg, "write error! socket_fd[%d], errno[%d], %s", socket_fd, errno, strerror(errno));
            return ERR_NOINIT;
        } else {
            done += ret;
        }
    }

    return 0;
}

sds exec_cmd(const char *fmt, ...)
{
    sds cmd = sdsnewlen(1024);
    if (cmd == NULL) {
        return NULL;
    }
    va_list ap;
    va_start(ap, fmt);
    int ret = vsdsprintf(cmd, fmt, ap);
    va_end(ap);
    if (ret < 0) {
        sdsfree(cmd);
        return NULL;
    }

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        sdsfree(cmd);
        return NULL;
    }
    sds s = load_file(fp);

    sdsfree(cmd);
    pclose(fp);
    return s;
}

int vsystem(const char *fmt, ...)
{
    sds cmd = sdsnewlen(1024);
    if (cmd == NULL) {
        return ERR_LIBCALL;
    }
    va_list ap;
    va_start(ap, fmt);
    int ret = vsdsprintf(cmd, fmt, ap);
    va_end(ap);
    if (ret < 0) {
        sdsfree(cmd);
        return ERR_LIBCALL;
    }

    ret = system(cmd);
    sdsfree(cmd);
    return ret;
}

int print_bin(const char *buf, int buflen/* = 0*/, FILE *fp/* = stdout*/)
{
    if (buf == NULL || buflen < 0) {
        return ERR_PARAM;
    }
    if (buflen == 0) {
        buflen = strlen(buf);
    }

    int col = 10;
    int row = buflen / col + (buflen % col ? 1 : 0);
    for (int i = 0; i < row; ++i) {
        fprintf(fp, "%010X: ", i);
        for (int j = 0; j < col; ++j) {
            int cnt = i * col + j;
            if (cnt >= buflen) {
                fprintf(fp, "  ");
            } else {
                fprintf(fp, "%02X", (unsigned char)buf[cnt]);
            }
            fprintf(fp, "%s", j == col - 1 ? "\t": " ");
        }
        for (int j = 0; j < col; ++j) {
            int cnt = i * col + j;
            if (cnt >= buflen) {
                fprintf(fp, " ");
            } else {
                fprintf(fp, "%c", isprint(buf[cnt]) ? buf[cnt] : '.');
            }
        }
        fprintf(fp, "\n");
    }
    return 0;
}

#ifdef COMMON_TOOL_TEST

int common_tool_test()
{
    sds s = load_file(__FILE__);
    if (s == NULL) { printf("load_file %s error! errno[%d], %s\n", __FILE__, errno, strerror(errno));
        return -1;
    }
    sdsprint(s);
    sdsfree(s);

    const char *file_name = "test_file";
    s = sdsnew("test_file");
    if (save_file(s, file_name) < 0) {
        printf("save file %s error!\n", file_name);
        return -1;
    } else {
        printf("save file %s success!\n", file_name);
    }
    sdsfree(s);

    FILE *fp = fopen(__FILE__, "r");
    while ((s = read_line(fp)) != NULL) {
        printf("%s\n", s);
        sdsfree(s);
    }
    fclose(fp);

    fp = fopen(__FILE__, "r");
    s = load_file(fp);
    printf("%s\n", s);
    sdsfree(s);
    fclose(fp);

    vsystem("echo nihao > 1");

    s = exec_cmd("ls");
    printf("%s\n", s);
    sdsfree(s);

    print_bin("nihao");

    return 0;
}

#endif      // COMMON_TOOL_TEST

}
