#ifndef __MINI_HEAP_H__
#define __MINI_HEAP_H__

#include <stddef.h>
#include <stdint.h>

typedef struct miniheap miniheap_t;
typedef struct heap_operations heap_operations_t;

struct heap_operations {
    void *(*alloc)(miniheap_t *heap, size_t size);
    void (*free)(miniheap_t *heap, void *ptr);
};

#define MINIHEAP_ALIGNMENT    (32)

typedef struct miniheap_freeblock {
    uint64_t size;
    struct miniheap_freeblock *next;
} miniheap_freeblock_t;

struct miniheap {
    void *base;
    size_t size;
    size_t used;
    uint32_t flags;
    miniheap_freeblock_t *free_list;
    heap_operations_t *ops;
};

extern struct miniheap g_miniheap;

void miniheap_init(struct miniheap *heap, void *base, size_t size);

#endif
