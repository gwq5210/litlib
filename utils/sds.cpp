/*************************************************************************
	> File Name: sds.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: Sat 23 Apr 2016 07:22:25 PM CST
 ************************************************************************/

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "sds.h"
#include "error.h"

namespace util {

sds sdsnew(const char *str/* = NULL */)
{
    return sdsnewlen(str, strlen(str));
}

sds sdsnewlen(int len)
{
    sds s = sdsnewlen(NULL, len);
    sdsclear(s);
    return s;
}

sds sdsnewlen(const void *buf, int len)
{
    if (len < 0) {
        return NULL;
    }

    int alloc_len = sizeof(sds_header) + len + 1;
    sds_header *sh = (sds_header *)jemalloc(alloc_len);
    if (sh == NULL) {
        return NULL;
    }

    memset(sh, 0, alloc_len);
    sh->magic = SDS_MAGIC;
    sh->len = len;
    sh->free = 0;

    if (buf && len) {
        memcpy(sh->buf, buf, len);
    }
    sh->buf[len] = '\0';

    return sh->buf;
}

sds sdsnewsds(const sds s)
{
    if (!sdscheck(s)) {
        return NULL;
    }
    return sdsnewlen(s, sdslen(s));
}

sds sdscpy(sds &s, const char *str)
{
    return sdscpylen(s, str, strlen(str));
}

sds sdscpylen(sds &s, const void *buf, int buf_len)
{
    int totlen = sdslen(s) + sdsavail(s);
    if (totlen < buf_len) {
        s = sdsgrow(s, buf_len - sdslen(s));
        if (s == NULL) {
            return NULL;
        }
        totlen = sdslen(s) + sdsavail(s);
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    sh->magic = SDS_MAGIC;
    memcpy(sh->buf, buf, buf_len); sh->buf[buf_len] = '\0';
    sh->len = buf_len;
    sh->free = totlen - buf_len;
    return s;
}

sds sdscpysds(sds &dst, const sds src)
{
    return sdscpylen(dst, src, sdslen(src));
}

sds sdscatlen(sds &s, const void *buf, int buf_len)
{
    if (!sdscheck(s)) {
        return NULL;
    }
    int len = sdslen(s);
    int avail = sdsavail(s);
    if (avail < buf_len) {
        s = sdsgrow(s, buf_len);
        if (s == NULL) {
            return NULL;
        }
        avail = sdsavail(s);
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    memcpy(s + len, buf, buf_len);
    sh->len += buf_len;
    sh->free -= buf_len;
    s[sh->len] = '\0';
    return s;
}

sds sdscat(sds &s, const char *str)
{
    return sdscatlen(s, str, strlen(str));
}

sds sdscatsds(sds &dst, const sds src)
{
    return sdscatlen(dst, src, sdslen(src));
}

sds sdstrim(sds &s, const char *cset/* = " \t\n"*/)
{
    sdsltrim(s, cset);
    sdsrtrim(s, cset);
    return s;
}

sds sdsltrim(sds &s, const char *cset/* = " \t\n"*/)
{
    if (sdsempty(s)) {
        return s;
    }

    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    int cset_len = strlen(cset);
    int len = sh->len;
    int cnt = 0;
    for (int i = 0; i < len; ++i) {
        int has = 0;
        for (int j = 0; j < cset_len; ++j) {
            if (s[i] == cset[j]) {
                has = 1;
                break;
            }
        }
        if (has == 0) {
            break;
        } else {
            ++cnt;
        }
    }
    sh->len -= cnt;
    sh->free += cnt;
    memmove(sh->buf, sh->buf + cnt, sh->len);
    s[sh->len] = '\0';
    return s;
}

sds sdsrtrim(sds &s, const char *cset/* = " \t"*/)
{
    if (sdsempty(s)) {
        return s;
    }

    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    int cset_len = strlen(cset);
    int len = sh->len;
    int cnt = 0;
    for (int i = len - 1; i >= 0; --i) {
        int has = 0;
        for (int j = 0; j < cset_len; ++j) {
            if (s[i] == cset[j]) {
                has = 1;
                break;
            }
        }
        if (has == 0) {
            break;
        } else {
            ++cnt;
        }
    }
    sh->len -= cnt;
    sh->free += cnt;
    s[sh->len] = '\0';
    return s;
}

sds *sdssplit(const sds s, const char *sep, int &count, int split_type/* = SPLIT_ALL*/)
{
    sds sds_sep = sdsnew(sep);
    sds *res = sdssplitlen(s, sds_sep, count, split_type);
    sdsfree(sds_sep);
    return res;
}

sds *sdssplitlen(const sds s, const sds sep, int &count, int split_type/* = SPLIT_ALL*/)
{
    if (s == NULL || sep == NULL) {
        count = 0;
        return NULL;
    }
    int len = sdslen(s);
    int sep_len = sdslen(sep);
    if (len < 0 || sep_len < 1) {
        count = 0;
        return NULL;
    }

    int guess_count = 10;
    int real_count = 0;
    sds *res = (sds *)jemalloc(sizeof(sds *) * guess_count);
    if (res == NULL) {
        return NULL;
    }
    memset(res, 0, sizeof(sds *) * guess_count);

    int start = 0;
    for (int i = 0; i < len - (sep_len - 1); ++i) {
        if (guess_count < real_count + 2) {
            guess_count *= 2;
            jefree(res);
            sds *new_res = (sds *)jerealloc(res, sizeof(sds *) * guess_count);
            if (res == NULL) {
                sdssplitfree(res, real_count);
                count = 0;
                return NULL;
            }
            res = new_res;
        }

        if ((sep_len == 1 && s[i] == sep[0]) || memcmp(&s[i], sep, sep_len) == 0) {
            if (split_type != SPLIT_LAST) {
                res[real_count] = sdsnewlen(s + start, i - start);
                if (res[real_count] == NULL) {
                    sdssplitfree(res, real_count);
                    count = 0;
                    return NULL;
                }
                ++real_count;
            }
            start = i + sep_len;
            i += sep_len - 1;
            if (split_type == SPLIT_FIRST) {
                break;
            }
        }
    }

    if (split_type == SPLIT_LAST) {
        res[real_count] = sdsnewlen(s, start - sep_len);
        if (res[real_count] == NULL) {
            sdssplitfree(res, real_count);
            count = 0;
            return NULL;
        }
        ++real_count;
    }
    res[real_count] = sdsnewlen(s + start, len - start);
    if (res[real_count] == NULL) {
        sdssplitfree(res, real_count);
        count = 0;
        return NULL;
    }
    ++real_count;
    count = real_count;
    return res;
}

int sdssplitfree(sds *&s, int count)
{
    if (s == NULL) {
        return 0;
    }
    for (int i = 0; i < count; ++i) {
        sdsfree(s[i]);
    }
    jefree(s);
    s = NULL;
    return 0;
}

void sdsclear(sds s)
{
    if (!sdscheck(s)) {
        return;
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    sh->free += sh->len;
    sh->len = 0;
    s[0] = '\0';
}

void sdsfree(sds &s)
{
    if (!sdscheck(s)) {
        return;
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    jefree(sh);
    s = NULL;
}

sds sdsgrow(sds &s, int addlen)
{
    if (!sdscheck(s)) {
        return NULL;
    }
    int avail = sdsavail(s);
    if (avail >= addlen) {
        return s;
    }
    int len = sdslen(s);
    int new_tot_len = len + addlen;
    if (len < SDS_MAX_PREMALLOC) {
        new_tot_len *= 2;
    } else {
        new_tot_len += SDS_MAX_PREMALLOC;
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    sds_header *new_sh = (sds_header *)jerealloc(sh, sizeof(sds_header) + new_tot_len + 1);
    if (new_sh == NULL) {
        jefree(sh);
        s = NULL;
        return NULL;
    }
    new_sh->free = new_tot_len - len;
    s = new_sh->buf;
    return s;
}

void sdsprint(const sds s, FILE *fp/* = stdout */)
{
    if (!sdscheck(s)) {
        fprintf(fp, "sds error!");
    }
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    fprintf(fp, "sh->magic = %X\n", sh->magic);
    fprintf(fp, "sh->len = %d\n", sh->len);
    fprintf(fp, "sh->free = %d\n", sh->free);
    if (sh->len == 0) {
        fprintf(fp, "sh->buf is empty!\n");
        return;
    }
    fprintf(fp, "s->buf:\n");
    int col = 10;
    int row = sh->len / col + (sh->len % col ? 1 : 0);
    for (int i = 0; i < row; ++i) {
        fprintf(fp, "%010X: ", i);
        for (int j = 0; j < col; ++j) {
            int cnt = i * col + j;
            if (cnt >= sh->len) {
                fprintf(fp, "  ");
            } else {
                fprintf(fp, "%02X", (unsigned char)s[cnt]);
            }
            fprintf(fp, "%s", j == col - 1 ? "\t": " ");
        }
        for (int j = 0; j < col; ++j) {
            int cnt = i * col + j;
            if (cnt >= sh->len) {
                fprintf(fp, " ");
            } else {
                fprintf(fp, "%c", isprint(s[cnt]) ? s[cnt] : '.');
            }
        }
        fprintf(fp, "\n");
    }
}

int sdsprintf(sds &s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsdsprintf(s, fmt, ap);
    va_end(ap);
    return ret;
}

int vsdsprintf(sds &s, const char *fmt, va_list ap)
{
    sdsclear(s);
    return vsdscatprintf(s, fmt, ap);
}

int sdscatprintf(sds &s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsdscatprintf(s, fmt, ap);
    va_end(ap);
    return ret;
}

int vsdscatprintf(sds &s, const char *fmt, va_list ap)
{
    char static_buf[1024];
    char *buf = static_buf;
    int buflen = strlen(fmt) * 2;
    int ret = 0;

    if (buflen > (int)sizeof(static_buf)) {
        buf = (char *)jemalloc(buflen);
        if (buf == NULL) {
            sdsfree(s);
            return ERR_ALLOC;
        }
    } else {
        buflen = sizeof(static_buf);
    }

    while (1) {
        va_list cpy;
        va_copy(cpy, ap);
        buf[buflen - 2] = '\0';
        ret = vsnprintf(buf, buflen, fmt, cpy);
        if (buf[buflen - 2] != '\0') {
            if (buf != static_buf) {
                jefree(buf);
            }
            buflen *= 2;
            buf = (char *)jemalloc(buflen);
            if (buf == NULL) {
                sdsfree(s);
                return ERR_ALLOC;
            }
            continue;
        }
        break;
    }

    s = sdscat(s, buf);
    if (buf != static_buf) {
        jefree(buf);
    }
    return ret;
}

#ifdef SDS_TEST

#include "test_helper.h"

int sds_test()
{
    sds s = sdsnewlen("nihao______fdajfkdsajfklj\0nifdhf", 32);        // 32
    sds_header *sh = (sds_header *)(s - sizeof(sds_header));
    test_cond("sds magic", sh->magic == SDS_MAGIC);
    test_cond("sdsnewlen(buf, len) len", sdslen(s) == 32);
    test_cond("sdsnewlen(buf, len) free", sdsavail(s) == 0);
    sdsfree(s);

    s = sdsnew("nihao");
    test_cond("sdsnew(\"nihao\")", strcmp(s, "nihao") == 0);

    s = sdscat(s, "nihao");
    test_cond("sdscat(s, \"nihao\")", strcmp(s, "nihaonihao") == 0);

    s = sdscatlen(s, "nih\t\nao", 7);
    test_cond("sdscatlen(s, \"nih\\t\\nao\", 7)", memcmp(s, "nihaonihaoni\t\nao", 17));
    sdsprint(s);

    sdsprintf(s, "nihao%d", 10);
    sdsprint(s);
    test_cond("sdsprintf(s, \"nihao%d\", 10)", strcmp(s, "nihao10") == 0);
    sdscatprintf(s, "nihao%d", 12);
    sdsprint(s);
    test_cond("sdsprintf(s, \"nihao%dnihao%d\", 10, 12)", strcmp(s, "nihao10nihao12") == 0);
    sdsfree(s);

    s = sdsnew("  \ta fdjak... \tinfd ka \t");
    sdstrim(s);
    test_cond("sdstrim(\"  \\ta fdjak... \\tinfd ka \\t\")", strcmp(s, "a fdjak... \tinfd ka") == 0);
    sdsfree(s);

    s = sdsnew("  \ta fdjak... \tinfd ka \t");
    sdstrim(s, " ");
    test_cond("sdstrim(\"  \\ta fdjak... \\tinfd ka \\t\", \" \")", strcmp(s, "\ta fdjak... \tinfd ka \t") == 0);
    sdsfree(s);

    s = sdsnew("  \ta fdjak... \tinfd ka \t");
    sdstrim(s, " \ta");
    test_cond("sdstrim(\"  \\ta fdjak... \\tinfd ka \\t\", \" \\ta\")", strcmp(s, "fdjak... \tinfd k") == 0);
    sdsfree(s);

    int count = 0;
    s = sdsnew("name = value #");
    sds *res = sdssplit(s, "=", count);
    test_cond("sdssplit(\"name = value #\", \"=\", count)", count == 2);
    test_cond("sdssplit(\"name = value #\", \"=\", count)", strcmp(res[0], "name ") == 0);
    test_cond("sdssplit(\"name = value #\", \"=\", count)", strcmp(res[1], " value #") == 0);
    for (int i = 0; i < count; ++i) {
        printf("%s\n", res[i]);
    }
    sdssplitfree(res, count);
    sdsfree(s);

    count = 0;
    s = sdsnew("name.=.value.#");
    res = sdssplit(s, s, count);
    test_cond("sdssplit(\"name.=.value.#\", \"name.=.value.#\", count)", count == 2);
    test_cond("sdssplit(\"name.=.value.#\", \"name.=.value.#\", count)", strcmp(res[0], "") == 0);
    test_cond("sdssplit(\"name.=.value.#\", \"name.=.value.#\", count)", strcmp(res[1], "") == 0);
    for (int i = 0; i < count; ++i) {
        printf("%s\n", res[i]);
    }
    sdssplitfree(res, count);
    sdsfree(s);

    count = 0;
    s = sdsnew("name.=.value.=#");
    res = sdssplit(s, "=", count, SPLIT_FIRST);
    test_cond("sdssplit(\"name.=.value.=#\", \"=\", count)", count == 2);
    test_cond("sdssplit(\"name.=.value.=#\", \"=\", count)", strcmp(res[0], "name.") == 0);
    test_cond("sdssplit(\"name.=.value.=#\", \"=\", count)", strcmp(res[1], ".value.=#") == 0);
    for (int i = 0; i < count; ++i) {
        printf("%s\n", res[i]);
    }
    sdssplitfree(res, count);
    res = sdssplit(s, "=", count, SPLIT_LAST);
    test_cond("sdssplit(\"name.=.value.=#\", \"=\", count)", count == 2);
    test_cond("sdssplit(\"name.=.value.=#\", \"=\", count)", strcmp(res[0], "name.=.value.") == 0);
    test_cond("sdssplit(\"name.=.value.=#\", \"=\", count)", strcmp(res[1], "#") == 0);
    for (int i = 0; i < count; ++i) {
        printf("%s\n", res[i]);
    }
    sdssplitfree(res, count);
    sdsfree(s);

    test_report();
    return 0;
}

#endif      // SDS_TEST

}       // namespace util
