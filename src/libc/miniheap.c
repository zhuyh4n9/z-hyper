#include "utils/miniheap.h"
#include "platform/aarch64_utils.h"
#include "platform/debug.h"

typedef struct miniheap_freeblock {
    size_t size;
    struct miniheap_freeblock *next;
} miniheap_freeblock_t;

void miniheap_init(struct miniheap *heap, void *base, size_t size)
{
    if (size == 0 || (size & (MINIHEAP_ALIGNMENT - 1))) {
        panic("miniheap_init: invalid size\n");
    }
    heap->base = base;
    heap->size = size;
    heap->used = 0;
    heap->flags = 0;
    heap->free_list = (miniheap_freeblock_t *)base;
    heap->free_list->size = size;
    heap->free_list->next = NULL;
}
