#ifndef __MINI_HEAP_H__
#define __MINI_HEAP_H__

#include <stddef.h>
#include <stdint.h>
#include <errno.h>

typedef struct miniheap miniheap_t;
typedef struct heap_operations heap_operations_t;

struct heap_operations {
    void *(*alloc)(miniheap_t *heap, size_t size);
    void (*free)(miniheap_t *heap, void *ptr);
};

#define MINIHEAP_ALIGNMENT    (16)

typedef struct miniheap_freeblock {
    uint32_t magic;
    uint32_t size;
    uint64_t next;
} miniheap_freeblock_t;

typedef struct miniheap_usedheader {
    uint32_t magic;
    uint32_t size;
} miniheap_usedheader_t;

struct miniheap {
    void *base;
    size_t size;
    size_t free_bytes;
    uint32_t align;
    miniheap_freeblock_t *free_list;
};
void dump_miniheap(miniheap_t *heap);
extern struct miniheap g_miniheap;

int miniheap_init(struct miniheap *heap, void *base, size_t size);

#endif
