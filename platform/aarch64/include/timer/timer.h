#ifndef __TIMER_H
#define __TIMER_H

#include "list/list.h"
#include "aarch64_utils.h"

void plat_timer_init(uint32_t interval_ms);

typedef struct percpu_timer_manager percpu_timer_manager_t;
typedef struct zhyper_timer zhyper_timer_t;

struct percpu_timer_manager {
    uint32_t nr_timers;
    uint64_t last_tick;
    struct list_head timer_list;
};

typedef void (*timer_callback_t)(zhyper_timer_t *timer, void *arg);

struct zhyper_timer {
    uint64_t expired_tick;
    uint32_t interval_ms;
    uint32_t is_periodic: 1;
    uint32_t is_active: 1;
    uint32_t reserved0: 30;
    timer_callback_t callback;
    void *arg;
    void *private;
    // simply use a list to manage timers, upgrade to skiplist in the future if needed
    struct list_head list;
};

zhyper_timer_t *create_timer_periodic(uint32_t interval_ms, timer_callback_t callback, void *arg);
zhyper_timer_t *create_timer_oneshot(uint32_t interval_ms, timer_callback_t callback, void *arg);
percpu_timer_manager_t *create_timer_manager(void);

int start_timer(zhyper_timer_t *timer, percpu_timer_manager_t *manager);

#endif
