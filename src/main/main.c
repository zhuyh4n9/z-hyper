#include "pl011/pl011.h"
#include "platform/aarch64_utils.h"
#include "platform/debug.h"
#include "utils/miniheap.h"
#include "layout.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void el2_exception_init(void);

void test_memset(void)
{
    char buffer[20];
    memset(buffer, 'A', sizeof(buffer));
    buffer[19] = '\0';
    printf("memset test: %s\n", buffer);
}

void test_memcpy(void)
{
    char src[] = "Hello, World!";
    char dest[20];
    memcpy(dest, src, strlen(src) + 1);
    printf("memcpy test: %s\n", dest);
}

void test_rand()
{
    srand(tick_el2());
    printf("rand test: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", rand());
    }
    printf("\n");
}

void stress_test_malloc()
{
    printf("----- Starting malloc stress test -----\n");
    void *ptrs[100];
    for (int i = 0; i < 100; i++) {
        size_t size = (rand() % 256) + 1; // Random size between 1 and 256 bytes
        ptrs[i] = malloc(size);
        if (ptrs[i]) {
            memset(ptrs[i], 0xAA, size); // Fill allocated memory with a pattern
        } else {
            printf("malloc failed at iteration %d\n", i);
        }
    }
    printf("Malloc stress test completed. Dumping miniheap state:\n");
    dump_miniheap(&g_miniheap);
    for (int i = 0; i < 100; i++) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }
    printf("miniheap info:\n");
    dump_miniheap(&g_miniheap);
    printf("----- Malloc stress test completed -----\n");
}

#define MAX_ALLOCATIONS 32
void malloc_fragmentation_test()
{
    printf("----- Starting malloc fragmentation test -----\n");
    void *ptrs[MAX_ALLOCATIONS];
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        uint32_t size = rand() % 1024 + 1; // Increasing size for each allocation
        ptrs[i] = malloc(size); // Allocate random size between 1 and 256 bytes
        if (ptrs[i]) {
            memset(ptrs[i], 0xBB, size); // Fill allocated memory with a pattern
        } else {
            printf("malloc failed at iteration %d, size: %u\n", i, size);
        }
        if (rand() % 2 == 0) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    dump_miniheap(&g_miniheap);
    // Free every other block to create fragmentation
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }
    printf("miniheap info after fragmentation\n");
    dump_miniheap(&g_miniheap);
    printf("----- Malloc fragmentation test completed -----\n");
}

extern uint64_t __heap_start;

int platform_init() {
    pl011_init(0x09000000);

    el2_exception_init();

    miniheap_init(&g_miniheap, (void *)&__heap_start, ZHYPER_HEAP_SIZE);

    printf("current EL: %d\n", get_current_el());
    printf("pstate: %x\n", pstate());
    printf("Platform initialized \n");
    // test_memset();
    // test_memcpy();
    test_rand();
    stress_test_malloc();
    malloc_fragmentation_test();
    panic("Test panic function");
    return 0;
}