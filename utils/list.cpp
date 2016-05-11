/*************************************************************************
	> File Name: list.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年05月10日 星期二 20时46分53秒
 ************************************************************************/

#include "list.h"
#include "common_tool.h"
#include "test_helper.h"

namespace util {

list::list(const list &other_list)
{
    init();
    push_back(other_list);
}

list &list::operator=(const list &other_list)
{
    clear();
    push_back(other_list);
}

list::~list()
{
    clear();
    sds s = (sds)header.head;
    sdsfree(s);
    s = (sds)header.tail;
    sdsfree(s);
    sdsfree(errmsg);
}

list_node *list::make_node(const void *val, int size)
{
    list_node *node = (list_node *)sdsnewlen(sizeof(struct list_node) + size);
    if (node == NULL) {
        set_errmsg(errmsg, "sdsnewlen error! node is NULL!");
        return NULL;
    }
    node->prev = NULL;
    node->next = NULL;
    memmove(node->val, val, size);
    sds s = (sds)node;
    sdssetlen(s, sizeof(struct list_node) + size);

    return node;
}

list_node *list::modify_node(list_node *old_node, const void *new_val, int new_size)
{
    sds s = (sds)old_node;
    int totlen = sdsavail(s) + sdslen(s);
    int new_totlen = new_size + sizeof(struct list_node);
    list_node *new_node = old_node;
    if (totlen < new_totlen) {
        new_node = make_node(new_val, new_size);
        if (new_node == NULL) {
            return NULL;
        }
        new_node->prev = old_node->prev;
        new_node->next = old_node->next;
        sdsfree(s);
    } else {
        memmove(new_node->val, new_val, new_size);
        sdssetlen(s, new_totlen);
    }

    return new_node;
}

int list::push_back(const list &other_list)
{
    int len = other_list.get_len();
    for (int i = 1; i <= len; ++i) {
        void *val = other_list.get(i);
        if (push_back(val, val_size(val)) < 0) {
            for (int j = 1; j < i; ++j) {
                pop_back();
            }
            return ERR_LIBCALL;
        }
    }
    return 0;
}

int list::push_front(const list &other_list)
{
    int len = other_list.get_len();
    for (int i = len; i >= 1; --i) {
        void *val = other_list.get(i);
        if (push_front(val, val_size(val)) < 0) {
            for (int j = 0; j < len - i; ++j) {
                pop_front();
            }
            return ERR_LIBCALL;
        }
    }
    return 0;
}

int list::insert(int pos, const void *val, int size)
{
    if (pos < 1 || pos > header.len + 1) {
        set_errmsg(errmsg, "param error! list len[%d], insert pos[%d]", header.len, pos);
        return ERR_PARAM;
    }

    if (val == NULL || size <= 0) {
        set_errmsg(errmsg, "param error! val[%p] is NULL or size[%d] less equal zero!", val, size);
        return ERR_PARAM;
    }

    list_node *new_node = make_node(val, size);
    if (new_node == NULL) {
        return ERR_LIBCALL;
    }

    list_node *node = LIST_NODE(get(pos - 1));
    list_node *cur = node->next;
    new_node->next = cur;
    new_node->prev = node;
    node->next = new_node;
    cur->prev = new_node;
    header.len++;

    return 0;
}

int list::insert(int pos, const list &other_list)
{
    if (pos < 1 || pos > header.len + 1) {
        set_errmsg(errmsg, "param error! list len[%d], insert pos[%d]", header.len, pos);
        return ERR_PARAM;
    }

    int len = other_list.get_len();
    for (int i = len; i >= 1; --i) {
        void *val = other_list.get(i);
        if (insert(pos, val, val_size(val)) < 0) {
            for (int j = 0; j < len - i; ++j) {
                remove(pos);
            }
            return ERR_LIBCALL;
        }
    }
    return 0;
}

int list::remove(int pos)
{
    if (empty()) {
        set_errmsg(errmsg, "list is empty!");
        return ERR_EMPTY;
    }

    if (pos < 1 || pos > header.len) {
        set_errmsg(errmsg, "param error! pos[%d], list len[%d]", pos, header.len);
        return ERR_PARAM;
    }

    list_node *node = LIST_NODE(get(pos - 1));
    list_node *cur = node->next;
    sds s = (sds)(node->next);
    node->next = cur->next;
    cur->next->prev = node;
    header.len--;
    sdsfree(s);
    return 0;
}

int list::modify(int pos, const void *val, int size)
{
    if (pos < 1 || pos > header.len) {
        set_errmsg(errmsg, "param error! list len[%d], insert pos[%d]", header.len, pos);
        return ERR_PARAM;
    }

    if (val == NULL || size <= 0) {
        set_errmsg(errmsg, "param error! val[%p] is NULL or size[%d] less equal zero!", val, size);
        return ERR_PARAM;
    }

    void *old_val = get(pos);
    list_node *old_node = LIST_NODE(old_val);
    list_node *new_node = modify_node(old_node, val, size);
    if (new_node == NULL) {
        return ERR_LIBCALL;
    }
    if (new_node != old_node) {
        new_node->prev->next = new_node;
        new_node->next->prev = new_node;
    }

    return 0;
}

int list::reverse()
{
    list_node *prev_node = header.head->prev;
    list_node *curr_node = header.head;
    list_node *next_node = curr_node->next;
    while (curr_node != NULL) {
        curr_node->next = prev_node;
        curr_node->prev = next_node;
        prev_node = curr_node;
        curr_node = next_node;
        if (next_node != NULL) {
            next_node = next_node->next;
        }
    }

    list_node *node = header.head;
    header.head = header.tail;
    header.tail = node;

    return 0;
}

void *list::get(int pos) const
{
    if (pos < 0 || pos > header.len + 1) {
        return NULL;
    }

    list_node *node = NULL;
    if (pos <= header.len / 2) {
        node = header.head;
        for (int i = 0; i < pos; ++i) {
            node = node->next;
        }
    } else {
        node = header.tail;
        for (int i = 0; i < header.len - pos + 1; ++i) {
            node = node->prev;
        }
    }

    if (node == NULL) {
        return NULL;
    }

    return node->val;
}

int list::clear()
{
    while (!empty()) {
        pop_back();
    }
    return 0;
}

int list::print(print_val_func disp_val, FILE *fp/* = stdout*/) const
{
    printf("list len %d\n", header.len);
    printf("prev[%p] <- node(0)[%p] -> next[%p]\n", header.head->prev, header.head, header.head->next);
    for (int i = 1; i <= header.len; ++i) {
        void *val = get(i);
        if (val == NULL) {
            printf("%s\n", get_errmsg());
            continue;
        }
        list_node *node = LIST_NODE(val);
        printf("prev[%p] <- node(%d)[%p] -> next[%p]\n", node->prev, i, node, node->next);
        disp_val(val);
    }
    printf("prev[%p] <- node(%d)[%p] -> next[%p]\n", header.tail->prev, header.len + 1, header.tail, header.tail->next);
    return 0;
}

#ifdef LIST_TEST

int print_int(void *val)
{
    printf("%d\n", *(int *)val);
    return 0;
}

int print_str(void *val)
{
    printf("%s\n", (char *)val);
    return 0;
}

int list_test()
{
    list nums;
    int num = 1;
    nums.print(print_int);
    nums.push_back(&num, sizeof(int));
    test_cond("push_back(1)", 1 == *(int *)nums.get(1));

    num = 2;
    nums.push_back(&num, sizeof(int));
    test_cond("push_back(1)", 1 == *(int *)nums.get(1));
    test_cond("push_back(2)", 2 == *(int *)nums.get(2));

    num = 3;
    nums.push_back(&num, sizeof(int));
    test_cond("push_back(1)", 1 == *(int *)nums.get(1));
    test_cond("push_back(2)", 2 == *(int *)nums.get(2));
    test_cond("push_back(3)", 3 == *(int *)nums.get(3));

    num = 4;
    nums.push_back(&num, sizeof(int));
    test_cond("push_back(1)", 1 == *(int *)nums.get(1));
    test_cond("push_back(2)", 2 == *(int *)nums.get(2));
    test_cond("push_back(3)", 3 == *(int *)nums.get(3));
    test_cond("push_back(4)", 4 == *(int *)nums.get(4));

    num = 5;
    nums.push_front(&num, sizeof(int));
    test_cond("push_back(1)", 5 == *(int *)nums.get(1));
    test_cond("push_back(2)", 1 == *(int *)nums.get(2));
    test_cond("push_back(3)", 2 == *(int *)nums.get(3));
    test_cond("push_back(4)", 3 == *(int *)nums.get(4));
    test_cond("push_front(5)", 4 == *(int *)nums.get(5));

    num = 6;
    nums.push_front(&num, sizeof(int));
    test_cond("push_back(1)", 6 == *(int *)nums.get(1));
    test_cond("push_back(2)", 5 == *(int *)nums.get(2));
    test_cond("push_back(3)", 1 == *(int *)nums.get(3));
    test_cond("push_back(4)", 2 == *(int *)nums.get(4));
    test_cond("push_front(5)", 3 == *(int *)nums.get(5));
    test_cond("push_front(6)", 4 == *(int *)nums.get(6));


    num = 7;
    nums.insert(2, &num, sizeof(int));
    test_cond("push_back(1)", 6 == *(int *)nums.get(1));
    test_cond("push_back(2)", 7 == *(int *)nums.get(2));
    test_cond("push_back(3)", 5 == *(int *)nums.get(3));
    test_cond("push_back(4)", 1 == *(int *)nums.get(4));
    test_cond("push_front(5)", 2 == *(int *)nums.get(5));
    test_cond("push_front(6)", 3 == *(int *)nums.get(6));
    test_cond("insert(1, 7)", 4 == *(int *)nums.get(7));

    num = 8;
    nums.insert(1, &num, sizeof(int));
    test_cond("push_back(1)", 8 == *(int *)nums.get(1));
    test_cond("push_back(2)", 6 == *(int *)nums.get(2));
    test_cond("push_back(3)", 7 == *(int *)nums.get(3));
    test_cond("push_back(4)", 5 == *(int *)nums.get(4));
    test_cond("push_front(5)", 1 == *(int *)nums.get(5));
    test_cond("push_front(6)", 2 == *(int *)nums.get(6));
    test_cond("insert(1, 7)", 3 == *(int *)nums.get(7));
    test_cond("insert(0, 8)", 4 == *(int *)nums.get(8));

    num = 9;
    nums.insert(9, &num, sizeof(int));
    test_cond("push_back(1)", 8 == *(int *)nums.get(1));
    test_cond("push_back(2)", 6 == *(int *)nums.get(2));
    test_cond("push_back(3)", 7 == *(int *)nums.get(3));
    test_cond("push_back(4)", 5 == *(int *)nums.get(4));
    test_cond("push_front(5)", 1 == *(int *)nums.get(5));
    test_cond("push_front(6)", 2 == *(int *)nums.get(6));
    test_cond("insert(1, 7)", 3 == *(int *)nums.get(7));
    test_cond("insert(0, 8)", 4 == *(int *)nums.get(8));
    test_cond("insert(8, 9)", 9 == *(int *)nums.get(9));

    nums.print(print_int);

    list names;
    names.push_back("nihao", strlen("nihao"));
    names.push_back("haha", strlen("haha"));
    names.push_back("heherf", strlen("heherf"));
    names.print(print_str);

    if (names.modify(1, "gwqaaa", strlen("gwqaaa")) < 0) {
        printf("modify failed! %s\n", names.get_errmsg());
        return -1;
    } else {
        printf("modify success!\n");
    }
    if (names.modify(2, "fdajkfdaj", strlen("fdajkfdaj")) < 0) {
        printf("modify failed! %s\n", names.get_errmsg());
        return -1;
    } else {
        printf("modify success!\n");
    }
    if (names.modify(3, "jfkdajfkdlaj", strlen("jfkdjafkldja")) < 0) {
        printf("modify failed! %s\n", names.get_errmsg());
        return -1;
    } else {
        printf("modify success!\n");
    }
    names.print(print_str);

    list stus(names);
    stus.print(print_str);

    stus.push_back(names);
    stus.print(print_str);

    stus = names;
    printf("no reverse\n");
    stus.print(print_str);
    stus.reverse();
    printf("reversed\n");
    stus.print(print_str);

    test_report();
    return 0;
}

#endif

}
