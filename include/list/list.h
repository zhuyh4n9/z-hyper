#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>
#include <stddef.h>

struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void list_init(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *node,
                              struct list_head *prev,
                              struct list_head *next)
{
    next->prev = node;
    node->next = next;
    node->prev = prev;
    prev->next = node;
}

static inline void list_add(struct list_head *node, struct list_head *head)
{
    __list_add(node, head, head->next);
}

static inline void list_add_tail(struct list_head *node, struct list_head *head)
{
    __list_add(node, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head *node)
{
    __list_del(node->prev, node->next);
    node->next = NULL;
    node->prev = NULL;
}

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (char *)(&((type *)0)->member)))

#define list_next(ptr) ((ptr)->next)
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev(ptr) ((ptr)->prev)
#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_entry(pos, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_entry(pos->member.next, typeof(*pos), member))

#endif