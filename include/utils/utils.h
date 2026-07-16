#ifndef __UTILS_H__
#define __UTILS_H__

// fixme
#include <atomic/atomic.h>
#include <spinlock/spinlock.h>

void panic(const char *fmt, ...);

typedef struct once_flag {
    atomic_t flag;
    spinlock_t lock;
} once_flag_t;

#define ONCE_INITIALIZER { .flag = ATOMIC_INIT(0), .lock = SPINLOCK_INITIALIZER }

static inline int run_once(once_flag_t *once, int (*func)(void))
{
    int ret = 0;
    smp_rmb();
    if (atomic_read(&once->flag) == 0) {
        spin_lock(&once->lock);
        if (atomic_read(&once->flag) == 0) {
            ret = func();
            if (ret == 0) {
                atomic_set(&once->flag, 1);
                smp_wmb();
            }
        }
        spin_unlock(&once->lock);
    }

    return ret;
}

#define RUN_ONCE(func)      \
    do { \
        static once_flag_t once = ONCE_INITIALIZER; \
        run_once(&once, func); \
    } while (0)

#endif
