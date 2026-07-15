#ifndef __AARCH64_BARRIAR_H_
#define __AARCH64_BARRIAR_H_

#define sev()		asm volatile("sev" : : : "memory")
#define wfe()		asm volatile("wfe" : : : "memory")
#define wfi()		asm volatile("wfi" : : : "memory")

#define isb()		asm volatile("isb" : : : "memory")
#define dsb(opt)	asm volatile("dsb sy" : : : "memory")

#define mb()		dsb()
#define rmb()		asm volatile("dsb ld" : : : "memory")
#define wmb()		asm volatile("dsb st" : : : "memory")


#define smp_mb()	asm volatile("dmb ish" : : : "memory")
#define smp_rmb()	asm volatile("dmb ishld" : : : "memory")
#define smp_wmb()	asm volatile("dmb ishst" : : : "memory")

#define __ACCESS_ONCE(x) ((volatile typeof(x) *)&(x))

#define ACCESS_ONCE(x) (*__ACCESS_ONCE(x))

#define smp_store_release(p, v)						\
do {									\
	compiletime_assert_atomic_type(*p);				\
	smp_mb();							\
	ACCESS_ONCE(*p) = (v);						\
} while (0)

#define smp_load_acquire(p)						\
({									\
	typeof(*p) ___p1 = ACCESS_ONCE(*p);				\
	compiletime_assert_atomic_type(*p);				\
	smp_mb();							\
	___p1;								\
})


#endif