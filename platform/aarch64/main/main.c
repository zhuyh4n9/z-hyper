#include "pl011/pl011.h"
#include "aarch64_utils.h"
#include "timer/timer.h"
#include "debug.h"
#include "utils/miniheap.h"
#include "layout.h"
#include "gic/gicv3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void miniheap_reliability_stress_test(void);
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

extern uint64_t __heap_start;

int platform_init() {
    pl011_init(0x09000000);

    el2_exception_init();
    enable_el2_irq();
    irq_enable();

    miniheap_init(&g_miniheap, (void *)&__heap_start, ZHYPER_HEAP_SIZE);

    printf("current EL: %d\n", get_current_el());
    printf("pstate: %x\n", pstate());
    printf("Platform initialized \n");

    test_rand();
    miniheap_reliability_stress_test();
    // fixme
    gicv3_init();
    gicv3_percpu_init();

    percpu_timer_init(1000); // 1 second interval
    printf("waiting for interrupts...\n");
    hang();

    return 0;
}