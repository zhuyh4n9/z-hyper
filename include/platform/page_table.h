#ifndef __PAGE_TABLE_H__
#define __PAGE_TABLE_H__

#include <stdint.h>

typedef uint64_t pte_t;
typedef uint64_t pmd_t;
typedef uint64_t pud_t;
typedef uint64_t pgd_t;
typedef uint64_t p4d_t;

#define VALID_BIT        (1ULL << 0)
#define TABLE_BIT        (1ULL << 1)


#endif
