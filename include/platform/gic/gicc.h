#ifndef __GICC_H__
#define __GICC_H__

#include <stdint.h>
#include "layout.h"

/*
 * GICv2 CPU Interface (GICC) Register Definitions
 * Memory-mapped at GICC_BASE (0x0F60000)
 * Accessible as a legacy interface even on GICv3.
 */

/* ========================================================================
 * Register Offsets
 * ======================================================================== */

#define GICC_CTLR        0x0000   /* CPU Interface Control Register */
#define GICC_PMR         0x0004   /* Interrupt Priority Mask Register */
#define GICC_BPR         0x0008   /* Binary Point Register */
#define GICC_IAR         0x000C   /* Interrupt Acknowledge Register */
#define GICC_EOIR        0x0010   /* End of Interrupt Register */
#define GICC_RPR         0x0014   /* Running Priority Register (read-only) */
#define GICC_HPPIR       0x0018   /* Highest Priority Pending Interrupt (read-only) */
#define GICC_ABPR        0x001C   /* Aliased Binary Point Register */
#define GICC_AIAR        0x0020   /* Aliased Interrupt Acknowledge Register */
#define GICC_AEOIR       0x0024   /* Aliased End of Interrupt Register */
#define GICC_AHPPIR      0x0028   /* Aliased Highest Priority Pending Interrupt (read-only) */
#define GICC_STATUSR     0x002C   /* Running Status (GICv2 only) */
#define GICC_APR0        0x00D0   /* Active Priorities Register 0 */
#define GICC_APR1        0x00D4   /* Active Priorities Register 1 */
#define GICC_APR2        0x00D8   /* Active Priorities Register 2 */
#define GICC_APR3        0x00DC   /* Active Priorities Register 3 */
#define GICC_NSAPR0      0x00E0   /* Non-secure Active Priorities Register 0 */
#define GICC_NSAPR1      0x00E4   /* Non-secure Active Priorities Register 1 */
#define GICC_NSAPR2      0x00E8   /* Non-secure Active Priorities Register 2 */
#define GICC_NSAPR3      0x00EC   /* Non-secure Active Priorities Register 3 */
#define GICC_IIDR        0x00FC   /* CPU Interface Identification Register */
#define GICC_DIR         0x1000   /* Deactivate Interrupt Register */

/* ========================================================================
 * GICC_CTLR bit definitions
 * ======================================================================== */
#define GICC_CTLR_ENABLE_GRP0     (1U << 0)   /* Enable Group 0 */
#define GICC_CTLR_ENABLE_GRP1     (1U << 1)   /* Enable Group 1 */
#define GICC_CTLR_ACKCTL          (1U << 2)   /* Group 0/1 acknowledge control */
#define GICC_CTLR_FIQEN           (1U << 3)   /* FIQ Enable */
#define GICC_CTLR_CBPR            (1U << 4)   /* Common BPR */
#define GICC_CTLR_FIQBYPDIS_GRP0  (1U << 5)   /* FIQ Bypass Disable Group 0 */
#define GICC_CTLR_IRQBYPDIS_GRP0  (1U << 6)   /* IRQ Bypass Disable Group 0 */
#define GICC_CTLR_FIQBYPDIS_GRP1  (1U << 7)   /* FIQ Bypass Disable Group 1 */
#define GICC_CTLR_IRQBYPDIS_GRP1  (1U << 8)   /* IRQ Bypass Disable Group 1 */
#define GICC_CTLR_EOIMODENS       (1U << 9)   /* Non-secure EOImode */

/* ========================================================================
 * GICC_IAR / GICC_AIAR field definitions
 * ======================================================================== */
#define GICC_IAR_INTID_MASK       0x3FF       /* Interrupt ID [9:0] */
#define GICC_IAR_CPUID_SHIFT      10          /* CPUID for SGI target */
#define GICC_IAR_CPUID_MASK       0x7

/* ========================================================================
 * GICC_EOIR / GICC_DIR field definitions
 * ======================================================================== */
#define GICC_EOIR_INTID_MASK      0x3FF       /* Interrupt ID [9:0] */

/* ========================================================================
 * GICC_PMR field definitions
 * ======================================================================== */
#define GICC_PMR_PRIORITY_MASK    0xFF        /* Priority mask [7:0] */

/* ========================================================================
 * GICC_BPR / GICC_ABPR field definitions
 * ======================================================================== */
#define GICC_BPR_BINARY_POINT_MASK 0x7        /* Binary point [2:0] */

/* ========================================================================
 * Special interrupt ID returned by IAR when no interrupt is pending
 * ======================================================================== */
#define GICC_SPURIOUS_INTID       1023

/* ========================================================================
 * Read/Write Macros
 * ======================================================================== */

#define GICC_REG32(offset)  (*(volatile uint32_t *)(GICC_BASE + (offset)))

static inline uint32_t gicc_read32(uint32_t offset)
{
    return GICC_REG32(offset);
}

static inline void gicc_write32(uint32_t offset, uint32_t val)
{
    GICC_REG32(offset) = val;
}

/* ========================================================================
 * Convenience Functions
 * ======================================================================== */

/* CTLR */
static inline uint32_t gicc_read_ctlr(void)
{
    return gicc_read32(GICC_CTLR);
}

static inline void gicc_write_ctlr(uint32_t val)
{
    gicc_write32(GICC_CTLR, val);
}

static inline void gicc_enable(void)
{
    gicc_write32(GICC_CTLR, gicc_read32(GICC_CTLR) | GICC_CTLR_ENABLE_GRP1);
}

static inline void gicc_disable(void)
{
    gicc_write32(GICC_CTLR, gicc_read32(GICC_CTLR) & ~GICC_CTLR_ENABLE_GRP1);
}

/* PMR */
static inline uint32_t gicc_read_pmr(void)
{
    return gicc_read32(GICC_PMR);
}

static inline void gicc_write_pmr(uint32_t priority)
{
    gicc_write32(GICC_PMR, priority & GICC_PMR_PRIORITY_MASK);
}

/* BPR */
static inline uint32_t gicc_read_bpr(void)
{
    return gicc_read32(GICC_BPR);
}

static inline void gicc_write_bpr(uint32_t val)
{
    gicc_write32(GICC_BPR, val & GICC_BPR_BINARY_POINT_MASK);
}

/* IAR — acknowledge interrupt, returns INTID */
static inline uint32_t gicc_read_iar(void)
{
    return gicc_read32(GICC_IAR);
}

static inline uint32_t gicc_iar_intid(uint32_t iar)
{
    return iar & GICC_IAR_INTID_MASK;
}

/* EOIR — end of interrupt */
static inline void gicc_write_eoir(uint32_t intid)
{
    gicc_write32(GICC_EOIR, intid & GICC_EOIR_INTID_MASK);
}

/* RPR — running priority (read-only) */
static inline uint32_t gicc_read_rpr(void)
{
    return gicc_read32(GICC_RPR);
}

/* HPPIR — highest priority pending (read-only) */
static inline uint32_t gicc_read_hppir(void)
{
    return gicc_read32(GICC_HPPIR);
}

/* ABPR */
static inline uint32_t gicc_read_abpr(void)
{
    return gicc_read32(GICC_ABPR);
}

static inline void gicc_write_abpr(uint32_t val)
{
    gicc_write32(GICC_ABPR, val & GICC_BPR_BINARY_POINT_MASK);
}

/* AIAR — aliased IAR */
static inline uint32_t gicc_read_aiar(void)
{
    return gicc_read32(GICC_AIAR);
}

/* AEOIR — aliased EOIR */
static inline void gicc_write_aeoir(uint32_t intid)
{
    gicc_write32(GICC_AEOIR, intid & GICC_EOIR_INTID_MASK);
}

/* AHPPIR — aliased HPPIR (read-only) */
static inline uint32_t gicc_read_ahppir(void)
{
    return gicc_read32(GICC_AHPPIR);
}

/* APR0~3 — Active Priority Registers (read-only) */
static inline uint32_t gicc_read_apr(uint32_t n)
{
    return gicc_read32(GICC_APR0 + ((n) & 0x3) * 4);
}

/* NSAPR0~3 — Non-secure Active Priority Registers (read-only) */
static inline uint32_t gicc_read_nsapr(uint32_t n)
{
    return gicc_read32(GICC_NSAPR0 + ((n) & 0x3) * 4);
}

/* IIDR */
static inline uint32_t gicc_read_iidr(void)
{
    return gicc_read32(GICC_IIDR);
}

/* DIR — Deactivate Interrupt Register */
static inline void gicc_write_dir(uint32_t intid)
{
    gicc_write32(GICC_DIR, intid & GICC_EOIR_INTID_MASK);
}

/* ========================================================================
 * Typical interrupt handling flow:
 *
 *   uint32_t iar = gicc_read_iar();
 *   uint32_t intid = gicc_iar_intid(iar);
 *   if (intid == GICC_SPURIOUS_INTID)
 *       return;                              // spurious - nothing to do
 *   // ... handle interrupt ...
 *   gicc_write_eoir(intid);                  // OR gicc_write_dir(intid) if EOImode
 * ======================================================================== */

#endif /* __GICC_H__ */
