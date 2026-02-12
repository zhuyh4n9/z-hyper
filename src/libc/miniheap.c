#include "utils/miniheap.h"
#include "platform/aarch64_utils.h"
#include "platform/debug.h"

#define MINIHEAP_MAGIC_FREE 0xDEADBEEF
#define MINIHEAP_MAGIC_USED 0xBEEFDEAD

int miniheap_init(struct miniheap *heap, void *base, size_t size)
{
    miniheap_freeblock_t *free_block = NULL;
    if (size == 0 || (size & (MINIHEAP_ALIGNMENT - 1))) {
        panic("miniheap_init: invalid size\n");
    }
    heap->base = base;
    heap->size = size;
    heap->free_bytes = size;
    heap->align = MINIHEAP_ALIGNMENT;
    
    free_block = (miniheap_freeblock_t *)base;
    free_block->magic = MINIHEAP_MAGIC_FREE;
    free_block->size = size;
    free_block->next = 0;
    heap->free_list = free_block;

    return 0;
}

static void *miniheap_alloc(miniheap_t *heap, size_t size)
{
    uint32_t total_size = (size + heap->align - 1) & ~(heap->align - 1);
    
    if (total_size == 0) {
        return NULL;
    }
    total_size += sizeof(miniheap_usedheader_t);
    if (total_size > heap->free_bytes) {
        return NULL;
    }

    miniheap_freeblock_t *prev = NULL;
    miniheap_freeblock_t *cur = heap->free_list;
    while (cur) {
        if (cur->size >= total_size) {
            //just fit
            if (cur->size == total_size) {
                if (prev) {
                    prev->next = cur->next;
                } else {
                    heap->free_list = (miniheap_freeblock_t *)(uintptr_t)cur->next;
                }
            } else {
                // split block
                miniheap_freeblock_t *new_block = (miniheap_freeblock_t *)((uintptr_t)cur + total_size);
                new_block->size = cur->size - total_size;
                new_block->next = cur->next;

                if (prev) {
                    prev->next = (uint64_t)new_block;
                } else {
                    heap->free_list = new_block;
                }
            }
            break;
        }
        prev = cur;
        cur = (miniheap_freeblock_t *)(uintptr_t)cur->next;
    }
    if (!cur) {
        return NULL;
    }
    heap->free_bytes -= total_size;
    
    miniheap_usedheader_t *used_header = (miniheap_usedheader_t *)cur;
    used_header->magic = MINIHEAP_MAGIC_USED;
    used_header->size = total_size;

    return (void *)((uintptr_t)cur + sizeof(miniheap_usedheader_t));
}

static void miniheap_free(miniheap_t *heap, void *ptr)
{
    if (!ptr) {
        return;
    }
    miniheap_usedheader_t *block = (miniheap_usedheader_t *)((uintptr_t)ptr - sizeof(miniheap_usedheader_t));

    if (block->magic != MINIHEAP_MAGIC_USED) {
        panic("miniheap_free: invalid free\n");
    }
    miniheap_freeblock_t *prev = NULL;
    miniheap_freeblock_t *cur = heap->free_list;
    uintptr_t ptr_rear = (uintptr_t)block + block->size;
    while (cur && (uintptr_t)cur < (uintptr_t)block) {
        if ((uintptr_t)cur >= ptr_rear) {
            break;
        }
        prev = cur;
        cur = (miniheap_freeblock_t *)(uintptr_t)cur->next;
    }

    uint32_t size = block->size;
    miniheap_freeblock_t *new_block = (miniheap_freeblock_t *)block;
    new_block->magic = MINIHEAP_MAGIC_FREE;
    if (prev && (uintptr_t)prev + prev->size == (uintptr_t)block) {
        prev->size += size;
        new_block = prev;
    } else {
        new_block->size = size;
        new_block->next = (uint64_t)cur;
        if (prev) {
            prev->next = (uint64_t)new_block;
        } else {
            heap->free_list = new_block;
        }
    }
    if (cur && (uintptr_t)ptr_rear == (uintptr_t)cur) {
        new_block->size += cur->size;
        new_block->next = cur->next;
    }
    heap->free_bytes += size;
}

void dump_miniheap(miniheap_t *heap)
{
    uint32_t i = 0;

    printf("Miniheap dump: base = %p, size = %u, free = %u\n", heap->base, heap->size, heap->free_bytes);
    miniheap_freeblock_t *cur = heap->free_list;
    while (cur) {
        printf("Free block[%d]: addr = %p, size = %u\n", i++, cur, cur->size);
        cur = (miniheap_freeblock_t *)(uintptr_t)cur->next;
    }
}

struct miniheap g_miniheap;

void *malloc(size_t size)
{
    return miniheap_alloc(&g_miniheap, size);
}

void free(void *ptr)
{
    miniheap_free(&g_miniheap, ptr);
}
