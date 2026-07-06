#define ZHIMGE_VADDR  ((paddr_t)0x40200000)

#define GICD_BASE     ((paddr_t)0x8000000)
#define GICR_BASE     ((paddr_t)0x80a0000)
#define GICC_BASE     ((paddr_t)0x0f60000)

#define MAX_SUPPORTED_CPUS          8

#define ZHYPER_HEAP_SIZE 0x100000 /* 1MB heap */