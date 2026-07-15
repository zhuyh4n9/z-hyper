#ifndef __ZHYPER_SPINLOCK_H__
#define __ZHYPER_SPINLOCK_H__

#include <spinlock/arch_spinlock.h>

typedef struct {
	arch_spinlock_t arch_lock;
} spinlock_t;

#define SPINLOCK_INITIALIZER { .arch_lock = __ARCH_SPIN_LOCK_UNLOCKED }
#define DEFINE_SPINLOCK(name) spinlock_t name = SPINLOCK_INITIALIZER

static inline void spin_lock_init(spinlock_t *lock)
{
	lock->arch_lock = (arch_spinlock_t)__ARCH_SPIN_LOCK_UNLOCKED;
}

static inline void spin_lock(spinlock_t *lock)
{
	arch_spin_lock(&lock->arch_lock);
}

static inline int spin_trylock(spinlock_t *lock)
{
	return arch_spin_trylock(&lock->arch_lock);
}

static inline void spin_unlock(spinlock_t *lock)
{
	arch_spin_unlock(&lock->arch_lock);
}

static inline int spin_is_locked(spinlock_t *lock)
{
	return arch_spin_is_locked(&lock->arch_lock);
}

static inline int spin_is_contended(spinlock_t *lock)
{
	return arch_spin_is_contended(&lock->arch_lock);
}

#endif /* __ZHYPER_SPINLOCK_H__ */
