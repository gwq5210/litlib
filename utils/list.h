/*************************************************************************
	> File Name: list.h
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月10日 星期二 20时46分59秒
 ************************************************************************/

#ifndef _LIST_H
#define _LIST_H

#include <string.h>

#include "sds.h"
#include "ret_status.h"

#define LIST_NODE(val) (list_node *)((char *)(val) - sizeof(struct list_node))

namespace util {

#pragma pack(1)

struct list_node {
    struct list_node *prev;
    struct list_node *next;
    char *val[];
};

#pragma pack()

struct list_header {
    int len;
    struct list_node *head;
    struct list_node *tail;
};

typedef int (*print_val_func)(void *);

class list {
public:
    list() { init(); }
    list(const list &other_list);
    list &operator=(const list &other_list);
    ~list();
    static int val_size(void *val)
    {
        sds s = (sds)LIST_NODE(val);
        return sdslen(s) - sizeof(struct list_node);
    }
    list_node *make_node(const void *val, int size);
    list_node *modify_node(list_node *old_node, const void *new_val, int new_size);
    int get_len() const { return header.len; }
    int push_back(const void *val, int size) { return insert(header.len + 1, val, size); }
    int push_front(const void *val, int size) { return insert(1, val, size); }
    int push_back(const list &other_list);
    int push_front(const list &other_list);
    int insert(int pos, const void *val, int size);
    int insert(int pos, const list &other_list);
    int remove(int pos);
    int modify(int pos, const void *val, int size);
    int modify_back(const void *val, int size) { return modify(header.len, val, size); }
    int modify_front(const void *val, int size) { return modify(1, val, size); }
    int pop_back() { return remove(header.len); }
    int pop_front() { return remove(1); }
    int reverse();
    void *get(int pos) const;
    void *get_head() const { return get(1); }
    void *get_tail() const { return get(header.len); }
    int clear();
    int empty() const { return header.len == 0; }
    int print(print_val_func disp_val, FILE *fp = stdout) const;
    const sds get_errmsg() const { return errmsg; }
private:
    int init()
    {
        memset(&header, 0, sizeof(list_header));
        header.head = (list_node *)sdsnewlen(sizeof(struct list_node));
        if (header.head == NULL) {
            return ERR_LIBCALL;
        }
        header.tail = (list_node *)sdsnewlen(sizeof(struct list_node));
        if (header.tail == NULL) {
            return ERR_LIBCALL;
        }
        header.head->next = header.tail;
        header.tail->prev = header.head;
        errmsg = sdsnewlen(1024);
        if (errmsg == NULL) {
            return ERR_LIBCALL;
        }
        return 0;
    }
    list_header header;
    sds errmsg;
};

#ifdef LIST_TEST

int list_test();

#endif

}

#endif      // _LIST_H
