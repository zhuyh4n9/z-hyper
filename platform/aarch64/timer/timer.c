#include "timer/timer.h"
#include "gic/gicv3.h"
#include <string.h>
#include "list/list.h"
#include "aarch64_utils.h"
#include <stdlib.h>
#include "debug.h"
#include <errno.h>

static volatile uint64_t g_counter = 0;

static percpu_timer_manager_t s_timer_manager[NR_CPU];

void percpu_timer_manager_init(percpu_timer_manager_t *manager)
{
    if (!manager) {
        panic("Timer manager is NULL\n");
    }
    manager->nr_timers = 0;
    manager->last_tick = tick_el2();
    list_init(&manager->timer_list);
}

static void reset_percpu_timer(void)
{
    write_sysreg(CNTHP_CTL_EL2, 0);
    write_sysreg(CNTHP_TVAL_EL2, 0);
    asm volatile ("isb");
}

static void set_percpu_timer(uint32_t interval_ms)
{
    uint64_t cntfrq_el0 = read_sysreg(CNTFRQ_EL0);
    uint64_t cntv_tval = (cntfrq_el0 / 1000) * interval_ms;

    write_sysreg(CNTHP_TVAL_EL2, cntv_tval);
    write_sysreg(CNTHP_CTL_EL2, 1);
    asm volatile ("isb");
}

static int timer_irq_handler(irq_context_t *irq, struct aarch64_gpregs *regs)
{
    (void)irq;
    (void)regs;

    percpu_timer_manager_t *manager = (percpu_timer_manager_t *)irq->arg;

    // Clear the timer interrupt
    reset_percpu_timer();

    // Call the user-defined callback if it exists
    ++g_counter;

    if (manager) {
        struct list_head *pos, *n;
        zhyper_timer_t *timer;

        list_for_each_safe(pos, n, &manager->timer_list) {
            timer = list_entry(pos, zhyper_timer_t, list);
            if (timer->is_active) {
                if (timer->callback) {
                    timer->callback(timer, timer->arg);
                }
                if (!timer->is_periodic) {
                    timer->is_active = 0;
                    list_del(&timer->list);
                    free(timer);
                }
            }
        }
    }
    set_percpu_timer(1000);
    return 0;
}

percpu_timer_manager_t *get_percpu_timer_manager(void)
{
    uint32_t id = cpu_id();
    if (id >= NR_CPU) {
        panic("CPU ID %u exceeds the maximum number of CPUs (%u)\n", id, NR_CPU);
    }
    return &s_timer_manager[id];
}

void percpu_timer_init(uint32_t interval_ms)
{
    ppi_context_t *timer_ppi = NULL;
    uint64_t cntfrq_el0 = read_sysreg(CNTFRQ_EL0);
    uint64_t cntv_tval = (cntfrq_el0 / 1000) * interval_ms;
    percpu_timer_manager_t *manager = NULL;

    printf("frequency: %lu Hz, interval: %u ms, cntv_tval: %lu\n", cntfrq_el0, interval_ms, cntv_tval);

    manager = get_percpu_timer_manager();
    if (!manager) {
        panic("Failed to get per-CPU timer manager\n");
    }
    timer_ppi = get_ppi_context(PPI_TIMER_INTID_EL2);
    if (timer_ppi == NULL) {
        panic("Failed to get PPI context for timer interrupt\n");
    }

    percpu_timer_manager_init(manager);

    timer_ppi->group = 1;
    timer_ppi->priority = 0x80;
    timer_ppi->trigger = 1;
    timer_ppi->handle = timer_irq_handler;
    timer_ppi->arg = manager;
    printf("Setting up PPI context for timer interrupt (INTID: %u)\n", PPI_TIMER_INTID_EL2);

    gic_irq_enable((irq_context_t *)timer_ppi);

    write_sysreg(CNTHP_TVAL_EL2, cntv_tval);
    write_sysreg(CNTHP_CTL_EL2, 1);
    asm volatile ("isb");
}


zhyper_timer_t *create_timer_periodic(uint32_t interval_ms, timer_callback_t callback, void *arg)
{
    zhyper_timer_t *timer = (zhyper_timer_t *)malloc(sizeof(zhyper_timer_t));
    if (!timer) {
        return NULL;
    }
    timer->interval_ms = interval_ms;
    timer->is_periodic = 1;
    timer->is_active = 0;
    timer->callback = callback;
    timer->arg = arg;
    timer->private = NULL;
    list_init(&timer->list);
    return timer;
}

zhyper_timer_t *create_timer_oneshot(uint32_t interval_ms, timer_callback_t callback, void *arg)
{
    zhyper_timer_t *timer = (zhyper_timer_t *)malloc(sizeof(zhyper_timer_t));
    if (!timer) {
        return NULL;
    }
    timer->interval_ms = interval_ms;
    timer->is_periodic = 0;
    timer->is_active = 0;
    timer->callback = callback;
    timer->arg = arg;
    timer->private = NULL;
    list_init(&timer->list);
    return timer;
}

int start_timer(zhyper_timer_t *timer, percpu_timer_manager_t *manager)
{
    if (!timer) {
        return -EINVAL;
    }
    if (manager == NULL) {
        manager = get_percpu_timer_manager();
        if (!manager) {
            return -EINVAL;
        }
    }

    timer->is_active = 1;
    manager->nr_timers++;
    list_add(&timer->list, &manager->timer_list);
    return 0;
}
