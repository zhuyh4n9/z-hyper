#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <aarch64_utils.h>
#include <timer/timer.h>
#include <debug.h>
#include <utils/miniheap.h>
#include <layout.h>
#include <gic/gicv3.h>
#include <utils/utils.h>
#include <utils/console.h>

extern uint64_t __heap_start;

static void test_timer_cb(zhyper_timer_t *timer, void *arg)
{
    (void)timer;
    (void)arg;
    static int s_count = 0;
    s_count++;
    printf("Timer callback invoked! Counter: %d\n", s_count);
}

static int __miniheap_init(void)
{
    miniheap_init(&g_miniheap, (void *)&__heap_start, ZHYPER_HEAP_SIZE);
    return 0;
}

int platform_init() {

    const console_t *pl011_console = NULL;
    static once_flag_t miniheap_init_once = ONCE_INITIALIZER;
    el2_exception_init();

    pl011_console = find_console("pl011");
    if (pl011_console) {
        console_init(pl011_console);
    } else {
        panic("pl011 console not found\n");
    }

    run_once(&miniheap_init_once, __miniheap_init);

    printf("Platform initialization started\n");


    plat_gicv3_init();
    plat_timer_init(1000/HZ);
    
    zhyper_timer_t *timer = create_timer_periodic(1000, test_timer_cb, NULL);
    start_timer(timer, NULL);

    printf("software EL: %d\n", get_current_el());

    printf("Platform initialized\n");

    while (1) {
        wfi();
    }

    return 0;
}