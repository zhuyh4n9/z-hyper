#include "utils/miniheap.h"
#include "platform/aarch64_utils.h"
#include "platform/debug.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define STRESS_SLOTS       128
#define STRESS_ITERATIONS  20000
#define STRESS_MAX_ALLOC   512

typedef struct stress_slot {
    void *ptr;
    uint32_t size;
    uint8_t pattern;
} stress_slot_t;

static stress_slot_t g_slots[STRESS_SLOTS];

static void fill_pattern(void *ptr, uint32_t size, uint8_t pattern)
{
    uint8_t *bytes = (uint8_t *)ptr;
    for (uint32_t i = 0; i < size; i++) {
        bytes[i] = pattern;
    }
}

static int verify_pattern(const void *ptr, uint32_t size, uint8_t pattern)
{
    const uint8_t *bytes = (const uint8_t *)ptr;
    for (uint32_t i = 0; i < size; i++) {
        if (bytes[i] != pattern) {
            return -1;
        }
    }
    return 0;
}

static int find_slot(int find_used)
{
    uint32_t start = (uint32_t)(rand() % STRESS_SLOTS);
    for (uint32_t i = 0; i < STRESS_SLOTS; i++) {
        uint32_t idx = (start + i) % STRESS_SLOTS;
        int used = (g_slots[idx].ptr != NULL);
        if ((find_used && used) || (!find_used && !used)) {
            return (int)idx;
        }
    }
    return -1;
}

void miniheap_reliability_stress_test(void)
{
    uint32_t alloc_ok = 0;
    uint32_t alloc_fail = 0;
    uint32_t free_ok = 0;
    uint32_t verify_ok = 0;

    for (uint32_t i = 0; i < STRESS_SLOTS; i++) {
        g_slots[i].ptr = NULL;
        g_slots[i].size = 0;
        g_slots[i].pattern = 0;
    }

    srand((unsigned int)(tick_el2() ^ 0xA5A55A5Au));

    printf("===== miniheap reliability stress test start =====\n");
    printf("heap size=%u free=%u slots=%u loops=%u\n",
           (uint32_t)g_miniheap.size,
           (uint32_t)g_miniheap.free_bytes,
           (uint32_t)STRESS_SLOTS,
           (uint32_t)STRESS_ITERATIONS);

    for (uint32_t iter = 0; iter < STRESS_ITERATIONS; iter++) {
        int do_alloc = (rand() % 100) < 65;

        if (do_alloc) {
            int slot = find_slot(0);
            if (slot < 0) {
                slot = find_slot(1);
                if (slot >= 0) {
                    if (verify_pattern(g_slots[slot].ptr, g_slots[slot].size, g_slots[slot].pattern) != 0) {
                        panic("miniheap stress: data corrupted before forced free");
                    }
                    verify_ok++;
                    free(g_slots[slot].ptr);
                    g_slots[slot].ptr = NULL;
                    g_slots[slot].size = 0;
                    g_slots[slot].pattern = 0;
                    free_ok++;
                }
            }

            if (slot >= 0) {
                uint32_t size = (uint32_t)(rand() % STRESS_MAX_ALLOC) + 1;
                uint8_t pattern = (uint8_t)(((iter + (uint32_t)slot) & 0x7F) + 1);
                void *ptr = malloc(size);
                if (ptr == NULL) {
                    alloc_fail++;
                } else {
                    fill_pattern(ptr, size, pattern);
                    g_slots[slot].ptr = ptr;
                    g_slots[slot].size = size;
                    g_slots[slot].pattern = pattern;
                    alloc_ok++;
                }
            }
        } else {
            int slot = find_slot(1);
            if (slot >= 0) {
                if (verify_pattern(g_slots[slot].ptr, g_slots[slot].size, g_slots[slot].pattern) != 0) {
                    panic("miniheap stress: data corrupted before free");
                }
                verify_ok++;
                free(g_slots[slot].ptr);
                g_slots[slot].ptr = NULL;
                g_slots[slot].size = 0;
                g_slots[slot].pattern = 0;
                free_ok++;
            }
        }

        if ((iter % 256u) == 0u) {
            int slot = find_slot(1);
            if (slot >= 0) {
                if (verify_pattern(g_slots[slot].ptr, g_slots[slot].size, g_slots[slot].pattern) != 0) {
                    panic("miniheap stress: periodic verify failed");
                }
                verify_ok++;
            }
            if (g_miniheap.free_bytes > g_miniheap.size) {
                panic("miniheap stress: invalid free_bytes");
            }
        }
    }

    for (uint32_t i = 0; i < STRESS_SLOTS; i++) {
        if (g_slots[i].ptr != NULL) {
            if (verify_pattern(g_slots[i].ptr, g_slots[i].size, g_slots[i].pattern) != 0) {
                panic("miniheap stress: data corrupted in cleanup");
            }
            verify_ok++;
            free(g_slots[i].ptr);
            g_slots[i].ptr = NULL;
            g_slots[i].size = 0;
            g_slots[i].pattern = 0;
            free_ok++;
        }
    }

    if (g_miniheap.free_bytes != g_miniheap.size) {
        printf("heap final state mismatch: size=%u free=%u\n",
               (uint32_t)g_miniheap.size,
               (uint32_t)g_miniheap.free_bytes);
        dump_miniheap(&g_miniheap);
        panic("miniheap stress: heap bytes not fully recovered");
    }

    printf("miniheap stress PASS: alloc_ok=%u alloc_fail=%u free_ok=%u verify_ok=%u\n",
           alloc_ok, alloc_fail, free_ok, verify_ok);
    dump_miniheap(&g_miniheap);
    printf("===== miniheap reliability stress test end =====\n");
}