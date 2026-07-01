#ifndef __GICR_H__
#define __GICR_H__

#include <stdint.h>
#include "layout.h"

/*
 * GICv3 Redistributor (GICR) Register Definitions
 * Memory-mapped at GICR_BASE (0x080A0000)
 * Each Redistributor has a 128KB (default) frame:
 *   - RD_base    (0x00000): Control registers
 *   - SGI_base   (0x10000): SGI/PPI configuration
 */

/* Redistributor frame stride (2 * 64KB) */
#define GICR_STRIDE              0x20000

/* Helper to compute per-processor redistributor base */
#define GICR_RD_BASE(cpu_id)     (GICR_BASE + (cpu_id) * GICR_STRIDE)
#define GICR_SGI_BASE(cpu_id)    (GICR_RD_BASE(cpu_id) + 0x10000)

/* ========================================================================
 * RD_base Register Offsets (relative to GICR_RD_BASE(cpu))
 * ======================================================================== */

#define GICR_CTLR                0x0000   /* Redistributor Control Register */
#define GICR_IIDR                0x0004   /* Implementer Identification Register */
#define GICR_TYPER               0x0008   /* Type Register (64-bit at 0x0008) */
#define GICR_STATUSR             0x0010   /* Error Reporting Status */
#define GICR_WAKER               0x0014   /* Wake Register */
#define GICR_MPAMIDR             0x0018   /* MPAM Information Register (64-bit) */
#define GICR_SETLPIR             0x0040   /* Set LPI Pending (64-bit) */
#define GICR_CLRLPIR             0x0048   /* Clear LPI Pending (64-bit) */
#define GICR_PROPBASER           0x0070   /* LPI Property Table Base Address (64-bit) */
#define GICR_PENDBASER           0x0078   /* LPI Pending Table Base Address (64-bit) */
#define GICR_INVLPIR             0x00A0   /* Invalidate LPI (64-bit) */
#define GICR_INVALLR             0x00B0   /* Invalidate All LPIs (64-bit) */
#define GICR_SYNCR               0x00C0   /* Synchronize Register */

/* ========================================================================
 * SGI_base Register Offsets (relative to GICR_SGI_BASE(cpu))
 * ======================================================================== */

#define GICR_IGROUPR0            0x0080   /* Interrupt Group Register 0 */
#define GICR_ISENABLER0          0x0100   /* Interrupt Set-Enable Register 0 */
#define GICR_ICENABLER0          0x0180   /* Interrupt Clear-Enable Register 0 */
#define GICR_ISPENDR0            0x0200   /* Interrupt Set-Pending Register 0 */
#define GICR_ICPENDR0            0x0280   /* Interrupt Clear-Pending Register 0 */
#define GICR_ISACTIVER0          0x0300   /* Interrupt Set-Active Register 0 */
#define GICR_ICACTIVER0          0x0380   /* Interrupt Clear-Active Register 0 */
#define GICR_IPRIORITYR(n)       (0x0400 + ((n) * 4))   /* Priority, n=0..7 (byte-accessible) */
#define GICR_ICFGR0              0x0C00   /* Interrupt Configuration Register 0 */
#define GICR_ICFGR1              0x0C04   /* Interrupt Configuration Register 1 */
#define GICR_IGRPMODR0           0x0D00   /* Interrupt Group Modifier Register 0 */
#define GICR_NSACR               0x0E00   /* Non-secure Access Control Register */

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

/* ========================================================================
 * GICR_PENDBASER bit definitions
 * ======================================================================== */
#define GICR_PENDBASER_PTZ               (1ULL << 62)   /* Pending Table Zero */
#define GICR_PENDBASER_PA_MASK           0x0000FFFFFFFFF000ULL

/* ========================================================================
 * GICR_SYNCR bit definitions
 * ======================================================================== */
#define GICR_SYNCR_COMPLETE              (1U << 0)      /* Operation complete */

/* ========================================================================
 * RD_base Read/Write Helpers (32-bit and 64-bit)
 * ======================================================================== */

#define GICR_RD_REG32(cpu_id, offset)  \
    (*(volatile uint32_t *)(GICR_RD_BASE(cpu_id) + (offset)))

#define GICR_RD_REG64(cpu_id, offset)  \
    (*(volatile uint64_t *)(GICR_RD_BASE(cpu_id) + (offset)))

/* SGI_base accessors */
#define GICR_SGI_REG32(cpu_id, offset) \
    (*(volatile uint32_t *)(GICR_SGI_BASE(cpu_id) + (offset)))

/* ========================================================================
 * RD_base Register Access Functions
 * ======================================================================== */

static inline uint32_t gicr_read32(uint32_t cpu_id, uint32_t offset)
{
    return GICR_RD_REG32(cpu_id, offset);
}

static inline void gicr_write32(uint32_t cpu_id, uint32_t offset, uint32_t val)
{
    GICR_RD_REG32(cpu_id, offset) = val;
}

static inline uint64_t gicr_read64(uint32_t cpu_id, uint32_t offset)
{
    return GICR_RD_REG64(cpu_id, offset);
}

static inline void gicr_write64(uint32_t cpu_id, uint32_t offset, uint64_t val)
{
    GICR_RD_REG64(cpu_id, offset) = val;
}

/* SGI_base accessors */
static inline uint32_t gicr_sgi_read32(uint32_t cpu_id, uint32_t offset)
{
    return GICR_SGI_REG32(cpu_id, offset);
}

static inline void gicr_sgi_write32(uint32_t cpu_id, uint32_t offset, uint32_t val)
{
    GICR_SGI_REG32(cpu_id, offset) = val;
}

/* ========================================================================
 * Convenience Functions — RD_base
 * ======================================================================== */

/* WAKER */
static inline void gicr_wake(uint32_t cpu_id)
{
    uint32_t val = gicr_read32(cpu_id, GICR_WAKER);
    val &= ~GICR_WAKER_PROCESSORSLEEP;
    gicr_write32(cpu_id, GICR_WAKER, val);
    /* Wait until the redistributor wakes up (ChildrenAsleep == 0) */
    while (gicr_read32(cpu_id, GICR_WAKER) & GICR_WAKER_CHILDRENASLEEP)
        ;
}

/* CTLR — enable LPIs */
static inline void gicr_enable_lpis(uint32_t cpu_id)
{
    uint32_t val = gicr_read32(cpu_id, GICR_CTLR);
    val |= GICR_CTLR_ENABLE_LPIS;
    gicr_write32(cpu_id, GICR_CTLR, val);
    /* Wait for RWP */
    while (gicr_read32(cpu_id, GICR_CTLR) & GICR_CTLR_RWP)
        ;
}

static inline void gicr_ctlr_wait_for_rwp(uint32_t cpu_id)
{
    while (gicr_read32(cpu_id, GICR_CTLR) & GICR_CTLR_RWP)
        ;
}

/* TYPER (64-bit at offset 0x0008) */
static inline uint64_t gicr_read_typer(uint32_t cpu_id)
{
    return gicr_read64(cpu_id, GICR_TYPER);
}

static inline uint32_t gicr_typer_affinity(uint32_t cpu_id)
{
    return (uint32_t)(gicr_read_typer(cpu_id) >> GICR_TYPER_AFFINITY_VALUE_SHIFT);
}

static inline int gicr_typer_is_last(uint32_t cpu_id)
{
    return (gicr_read_typer(cpu_id) & GICR_TYPER_LAST) != 0;
}

/* PROPBAER / PENDBASER (64-bit) */
static inline uint64_t gicr_read_propbaser(uint32_t cpu_id)
{
    return gicr_read64(cpu_id, GICR_PROPBASER);
}

static inline void gicr_write_propbaser(uint32_t cpu_id, uint64_t val)
{
    gicr_write64(cpu_id, GICR_PROPBASER, val);
}

static inline uint64_t gicr_read_pendbaser(uint32_t cpu_id)
{
    return gicr_read64(cpu_id, GICR_PENDBASER);
}

static inline void gicr_write_pendbaser(uint32_t cpu_id, uint64_t val)
{
    gicr_write64(cpu_id, GICR_PENDBASER, val);
}

/* SETLPIR / CLRLPIR / INVLPIR / INVALLR (64-bit) */
static inline void gicr_set_lpi_pending(uint32_t cpu_id, uint64_t intid)
{
    gicr_write64(cpu_id, GICR_SETLPIR, intid);
}

static inline void gicr_clear_lpi_pending(uint32_t cpu_id, uint64_t intid)
{
    gicr_write64(cpu_id, GICR_CLRLPIR, intid);
}

static inline void gicr_inv_lpi(uint32_t cpu_id, uint64_t intid)
{
    gicr_write64(cpu_id, GICR_INVLPIR, intid);
}

static inline void gicr_inv_all_lpis(uint32_t cpu_id, uint64_t intid)
{
    gicr_write64(cpu_id, GICR_INVALLR, intid);
}

/* SYNCR */
static inline void gicr_wait_for_sync(uint32_t cpu_id)
{
    while (!(gicr_read32(cpu_id, GICR_SYNCR) & GICR_SYNCR_COMPLETE))
        ;
}

/* ========================================================================
 * Convenience Functions — SGI_base (SGI/PPI configuration)
 * ======================================================================== */

/* IGROUPR0 */
static inline uint32_t gicr_read_igroupr0(uint32_t cpu_id)
{
    return gicr_sgi_read32(cpu_id, GICR_IGROUPR0);
}

static inline void gicr_write_igroupr0(uint32_t cpu_id, uint32_t val)
{
    gicr_sgi_write32(cpu_id, GICR_IGROUPR0, val);
}

/* Set/Clear Enable */
static inline void gicr_set_enable(uint32_t cpu_id, uint32_t intid)
{
    gicr_sgi_write32(cpu_id, GICR_ISENABLER0, (1U << intid));
}

static inline void gicr_clear_enable(uint32_t cpu_id, uint32_t intid)
{
    gicr_sgi_write32(cpu_id, GICR_ICENABLER0, (1U << intid));
}

static inline uint32_t gicr_read_enable(uint32_t cpu_id)
{
    return gicr_sgi_read32(cpu_id, GICR_ISENABLER0);
}

/* Set/Clear Pending */
static inline void gicr_set_pending(uint32_t cpu_id, uint32_t intid)
{
    gicr_sgi_write32(cpu_id, GICR_ISPENDR0, (1U << intid));
}

static inline void gicr_clear_pending(uint32_t cpu_id, uint32_t intid)
{
    gicr_sgi_write32(cpu_id, GICR_ICPENDR0, (1U << intid));
}

static inline uint32_t gicr_read_pending(uint32_t cpu_id)
{
    return gicr_sgi_read32(cpu_id, GICR_ISPENDR0);
}

/* Set/Clear Active */
static inline void gicr_set_active(uint32_t cpu_id, uint32_t intid)
{
    gicr_sgi_write32(cpu_id, GICR_ISACTIVER0, (1U << intid));
}

static inline void gicr_clear_active(uint32_t cpu_id, uint32_t intid)
{
    gicr_sgi_write32(cpu_id, GICR_ICACTIVER0, (1U << intid));
}

/* Priority (byte-accessible, 8 registers for INTID 0..31) */
static inline void gicr_write_priority(uint32_t cpu_id, uint32_t intid, uint8_t priority)
{
    uint32_t n = intid >> 2;
    uint32_t shift = (intid & 0x3) * 8;
    uint32_t val = gicr_sgi_read32(cpu_id, GICR_IPRIORITYR(n));
    val &= ~(0xFFU << shift);
    val |= ((uint32_t)priority << shift);
    gicr_sgi_write32(cpu_id, GICR_IPRIORITYR(n), val);
}

static inline uint8_t gicr_read_priority(uint32_t cpu_id, uint32_t intid)
{
    uint32_t n = intid >> 2;
    uint32_t shift = (intid & 0x3) * 8;
    return (uint8_t)((gicr_sgi_read32(cpu_id, GICR_IPRIORITYR(n)) >> shift) & 0xFF);
}

/* Configuration (2 bits per interrupt: ICFGR0 for INTID 0..15, ICFGR1 for 16..31) */
static inline void gicr_write_cfg(uint32_t cpu_id, uint32_t intid, uint32_t cfg)
{
    uint32_t offset = (intid < 16) ? GICR_ICFGR0 : GICR_ICFGR1;
    uint32_t shift = ((intid & 0xF) * 2);
    uint32_t val = gicr_sgi_read32(cpu_id, offset);
    val &= ~(0x3U << shift);
    val |= (cfg & 0x3) << shift;
    gicr_sgi_write32(cpu_id, offset, val);
}

/* IGRPMODR0 */
static inline uint32_t gicr_read_igrpmodr0(uint32_t cpu_id)
{
    return gicr_sgi_read32(cpu_id, GICR_IGRPMODR0);
}

static inline void gicr_write_igrpmodr0(uint32_t cpu_id, uint32_t val)
{
    gicr_sgi_write32(cpu_id, GICR_IGRPMODR0, val);
}

/* NSACR */
static inline uint32_t gicr_read_nsacr(uint32_t cpu_id)
{
    return gicr_sgi_read32(cpu_id, GICR_NSACR);
}

/* ========================================================================
 * Initialization helper — wake and configure a Redistributor
 * ======================================================================== */
static inline void gicr_init(uint32_t cpu_id)
{
    gicr_wake(cpu_id);
    /* Configure Group 0 as Group 1 Non-secure for all SGI/PPI */
    gicr_write_igroupr0(cpu_id, ~0U);
}

#endif /* __GICR_H__ */
