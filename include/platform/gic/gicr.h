#ifndef __GICR_H__
#define __GICR_H__

#include <stddef.h>
#include <stdint.h>
#include "layout.h"

struct gicv3_redistributor_regs {
    // 0x0 ~ 0x10
    uint32_t ctlr;
    uint32_t iidr;
    uint64_t typer;
    // 0x10 ~ 0x20
    uint32_t statusr;
    uint32_t waker;
    uint32_t reserved0[2];
    // 0x20 ~ 0x40  (GICv3: RES0)
    uint32_t reserved_0020_003f[8];
    // 0x40 ~ 0x50
    uint64_t setlpir;
    uint64_t clrlpir;
    // 0x50 ~ 0x70
    uint32_t reserved1[8];
    // 0x70 ~ 0x80
    uint64_t propbaser;
    uint64_t pendbaser;
    // 0x80 ~ 0xA0
    uint32_t reserved2[8];
    // 0xA0 ~ 0xB0
    uint64_t invlpir;
    uint32_t reserved3[2];
    // 0xB0 ~ 0xC0
    uint64_t invallr;
    uint32_t reserved4[2];
    // 0xC0 ~ 0x10000
    uint32_t syncr;
    uint32_t reserved_00c4_10000[0x3FCF];
};

/* Static assertions — validate register offsets against GICv3 spec */
_Static_assert(sizeof(struct gicv3_redistributor_regs) == 0x10000, "GICR struct size must be 64KB");
_Static_assert(offsetof(struct gicv3_redistributor_regs, ctlr)       == 0x0000, "GICR_CTLR offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, iidr)       == 0x0004, "GICR_IIDR offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, typer)      == 0x0008, "GICR_TYPER offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, statusr)    == 0x0010, "GICR_STATUSR offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, waker)      == 0x0014, "GICR_WAKER offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, setlpir)    == 0x0040, "GICR_SETLPIR offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, clrlpir)    == 0x0048, "GICR_CLRLPIR offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, propbaser)  == 0x0070, "GICR_PROPBASER offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, pendbaser)  == 0x0078, "GICR_PENDBASER offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, invlpir)    == 0x00A0, "GICR_INVLPIR offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, invallr)    == 0x00B0, "GICR_INVALLR offset");
_Static_assert(offsetof(struct gicv3_redistributor_regs, syncr)      == 0x00C0, "GICR_SYNCR offset");

struct gicv3_redistributor_sgi_regs {
    // 0x0 ~ 0x80
    uint32_t reserved0[32];
    // 0x80 ~ 0x100
    uint32_t igroup0;
    uint32_t reserved1[31];
    // 0x100 ~ 0x180
    uint32_t isenabler0;
    uint32_t reserved2[31];
    // 0x180 ~ 0x200
    uint32_t icenabler0;
    uint32_t reserved3[31];
    // 0x200 ~ 0x280
    uint32_t ispendr0;
    uint32_t reserved4[31];
    // 0x280 ~ 0x300
    uint32_t icpendr0;
    uint32_t reserved5[31];
    // 0x300 ~ 0x380
    uint32_t isactiver0;
    uint32_t reserved6[31];
    // 0x380 ~ 0x400
    uint32_t icactiver0;
    uint32_t reserved7[31];
    // 0x400 ~ 0x420
    uint32_t ipriorityr[8];
    // 0x420 ~ 0xC00
    uint32_t reserved8[0x1F8];
    // 0xC00 ~ 0xC08
    uint32_t icfgr0;
    uint32_t icfgr1;
    // 0xC08 ~ 0xD00
    uint32_t reserved9[62];
    // 0xD00 ~ 0xE00
    uint32_t igrpmodr0;
    uint32_t reserved10[63];
    // 0xE00 ~ 0x10000
    uint32_t gicr_nsacr;
    uint32_t reserved11[0x3C7F];
};

/* Static assertions — validate SGI register offsets against GICv3 spec */
_Static_assert(sizeof(struct gicv3_redistributor_sgi_regs) == 0x10000, "GICR SGI struct size must be 64KB");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, igroup0)    == 0x0080, "GICR_IGROUPR0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, isenabler0) == 0x0100, "GICR_ISENABLER0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, icenabler0) == 0x0180, "GICR_ICENABLER0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, ispendr0)   == 0x0200, "GICR_ISPENDR0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, icpendr0)   == 0x0280, "GICR_ICPENDR0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, isactiver0) == 0x0300, "GICR_ISACTIVER0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, icactiver0) == 0x0380, "GICR_ICACTIVER0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, ipriorityr) == 0x0400, "GICR_IPRIORITYR offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, icfgr0)     == 0x0C00, "GICR_ICFGR0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, icfgr1)     == 0x0C04, "GICR_ICFGR1 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, igrpmodr0)  == 0x0D00, "GICR_IGRPMODR0 offset");
_Static_assert(offsetof(struct gicv3_redistributor_sgi_regs, gicr_nsacr) == 0x0E00, "GICR_NSACR offset");

#define GICR_STRIDE              0x20000

/* ========================================================================
 * GICR_CTLR bit definitions
 * ======================================================================== */
#define GICR_CTLR_ENABLE_LPIS         (1U << 0)    /* Enable LPIs */
#define GICR_CTLR_RWP                 (1U << 3)    /* Register Write Pending */
#define GICR_CTLR_UWP                 (1U << 31)   /* Update Write Pending */

/* ========================================================================
 * GICR_WAKER bit definitions
 * ======================================================================== */
#define GICR_WAKER_PROCESSORSLEEP     (1U << 1)    /* Processor Sleep */
#define GICR_WAKER_CHILDRENASLEEP     (1U << 2)    /* Children Asleep */

/* ========================================================================
 * GICR_TYPER bit definitions (64-bit)
 * ======================================================================== */
#define GICR_TYPER_AFFINITY_VALUE_SHIFT  32
#define GICR_TYPER_AFFINITY_VALUE_MASK   0xFFFFFFFFULL
#define GICR_TYPER_PROCESSOR_NUMBER_SHIFT 8
#define GICR_TYPER_PROCESSOR_NUMBER_MASK  0xFFFF
#define GICR_TYPER_LAST                  (1ULL << 4)    /* Last Redistributor in series */
#define GICR_TYPER_DIRTY                 (1ULL << 5)    /* RD has LPIs pending */
#define GICR_TYPER_VLPI                  (1ULL << 6)    /* VLPI support */
#define GICR_TYPER_PLPI                  (1ULL << 7)    /* Physical LPI support */
#define GICR_TYPER_DPG0                  (1ULL << 12)   /* Disable pseudo-NMI Group 0 */
#define GICR_TYPER_DPG1S                 (1ULL << 13)   /* Disable pseudo-NMI Group 1 Secure */
#define GICR_TYPER_DPG1NS                (1ULL << 14)   /* Disable pseudo-NMI Group 1 Non-secure */
#define GICR_TYPER_RSS                   (1ULL << 26)   /* Range Selector Support */
#define GICR_TYPER_NOCLUSTER             (1ULL << 63)   /* No inner-cacheability sharing */

/* ========================================================================
 * GICR_PROPBASER bit definitions
 * ======================================================================== */
#define GICR_PROPBASER_IDBITS_SHIFT      0
#define GICR_PROPBASER_IDBITS_MASK       0x1F
#define GICR_PROPBASER_SHAREABILITY_SHIFT 10
#define GICR_PROPBASER_SHAREABILITY_MASK  0x3
#define GICR_PROPBASER_OUTER_CACHE_SHIFT  56
#define GICR_PROPBASER_OUTER_CACHE_MASK   0x7
#define GICR_PROPBASER_INNER_CACHE_SHIFT  7
#define GICR_PROPBASER_INNER_CACHE_MASK   0x7
#define GICR_PROPBASER_PA_MASK           0x0000FFFFFFFFF000ULL


#define GICR_PENDBASER_PTZ               (1ULL << 62)   /* Pending Table Zero */
#define GICR_PENDBASER_PA_MASK           0x0000FFFFFFFFF000ULL

#define GICR_SYNCR_COMPLETE              (1U << 0)      /* Operation complete */

typedef struct gicd_context gicd_context_t;
typedef struct gicr_context gicr_context_t;

int gicr_get(gicd_context_t *gicd_ctx,uint32_t cpu_id);

int gicr_init(gicr_context_t *gicr_ctx);

bool gicr_uwp(gicr_context_t *gicr_ctx);
bool gicr_rwp(gicr_context_t *gicr_ctx);

#endif
