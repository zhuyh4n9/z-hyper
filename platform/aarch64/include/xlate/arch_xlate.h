#ifndef __ARCH_XLATE_H__
#define __ARCH_XLATE_H__

#include <stdint.h>

typedef uint64_t pte_t;
typedef uint64_t pmd_t;
typedef uint64_t pud_t;
typedef uint64_t pgd_t;
typedef uint64_t p4d_t;

#define VALID_BIT        (1ULL << 0)
#define TABLE_BIT        (1ULL << 1)



#endif
