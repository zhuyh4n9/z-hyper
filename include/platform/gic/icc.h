#ifndef __ICC_H__
#define __ICC_H__

#include <stdint.h>
#include "platform/aarch64_utils.h"

/*
 * GICv3 CPU Interface Registers (System Register access via MRS/MSR)
 *
 * These are ICC_*_EL1 registers accessible from EL2.
 * Use ICC_SRE_EL2 to enable system register interface before use.
 */

/* ========================================================================
 * ICC_SRE_EL2 — System Register Enable (EL2)
 * ======================================================================== */
#define ICC_SRE_EL2_SRE     (1U << 0)   /* System Register Enable */
#define ICC_SRE_EL2_DFB     (1U << 1)   /* Disable FIQ Bypass */
#define ICC_SRE_EL2_DIB     (1U << 2)   /* Disable IRQ Bypass */
#define ICC_SRE_EL2_Enable  (1U << 3)   /* Enable EL2 access to ICC_SRE_EL1 */

static inline uint64_t icc_read_sre_el2(void)
{
    return read_sysreg(ICC_SRE_EL2);
}

static inline void icc_write_sre_el2(uint64_t val)
{
    write_sysreg(ICC_SRE_EL2, val);
}

/* ========================================================================
 * ICC_SRE_EL1 — System Register Enable (EL1)
 * ======================================================================== */
#define ICC_SRE_EL1_SRE     (1U << 0)   /* System Register Enable */
#define ICC_SRE_EL1_DFB     (1U << 1)   /* Disable FIQ Bypass */
#define ICC_SRE_EL1_DIB     (1U << 2)   /* Disable IRQ Bypass */

/* ========================================================================
 * ICC_CTLR_EL1 — Control Register
 * ======================================================================== */
#define ICC_CTLR_EL1_EOImode            (1U << 1)   /* EOImode: 0 = EOIR, 1 = DIR deactivation */
#define ICC_CTLR_EL1_CBPR               (1U << 0)   /* Common BPR */
#define ICC_CTLR_EL1_PRI_BITS_SHIFT     8           /* Priority bits field shift */
#define ICC_CTLR_EL1_PRI_BITS_MASK      0x7         /* Priority bits field mask */
#define ICC_CTLR_EL1_ID_BITS_SHIFT      11          /* ID bits field shift */
#define ICC_CTLR_EL1_ID_BITS_MASK       0x7         /* ID bits field mask */
#define ICC_CTLR_EL1_SEIS               (1U << 14)  /* SEI Support */
#define ICC_CTLR_EL1_A3V                (1U << 15)  /* Affinity level 3 valid */
#define ICC_CTLR_EL1_RSS                (1U << 18)  /* Range Selector Support */
#define ICC_CTLR_EL1_ExtRange           (1U << 19)  /* Extended INTID range */

static inline uint64_t icc_read_ctlr_el1(void)
{
    return read_sysreg(ICC_CTLR_EL1);
}

static inline void icc_write_ctlr_el1(uint64_t val)
{
    write_sysreg(ICC_CTLR_EL1, val);
}

/* ========================================================================
 * ICC_PMR_EL1 — Priority Mask Register
 * ======================================================================== */
#define ICC_PMR_PRIORITY_MASK   0xFF    /* Priority mask [7:0] */

static inline uint64_t icc_read_pmr_el1(void)
{
    return read_sysreg(ICC_PMR_EL1);
}

static inline void icc_write_pmr_el1(uint64_t val)
{
    write_sysreg(ICC_PMR_EL1, val);
}

/* ========================================================================
 * ICC_BPR0_EL1 — Binary Point Register Group 0
 * ======================================================================== */
#define ICC_BPR0_BINARY_POINT_MASK  0x7 /* Binary point [2:0] */

static inline uint64_t icc_read_bpr0_el1(void)
{
    return read_sysreg(ICC_BPR0_EL1);
}

static inline void icc_write_bpr0_el1(uint64_t val)
{
    write_sysreg(ICC_BPR0_EL1, val);
}

/* ========================================================================
 * ICC_BPR1_EL1 — Binary Point Register Group 1
 * ======================================================================== */
#define ICC_BPR1_BINARY_POINT_MASK  0x7 /* Binary point [2:0] */

static inline uint64_t icc_read_bpr1_el1(void)
{
    return read_sysreg(ICC_BPR1_EL1);
}

static inline void icc_write_bpr1_el1(uint64_t val)
{
    write_sysreg(ICC_BPR1_EL1, val);
}

/* ========================================================================
 * ICC_IGRPEN0_EL1 — Interrupt Group 0 Enable
 * ======================================================================== */
#define ICC_IGRPEN0_ENABLE  (1U << 0)   /* Group 0 enable */

static inline uint64_t icc_read_igrpen0_el1(void)
{
    return read_sysreg(ICC_IGRPEN0_EL1);
}

static inline void icc_write_igrpen0_el1(uint64_t val)
{
    write_sysreg(ICC_IGRPEN0_EL1, val);
}

/* ========================================================================
 * ICC_IGRPEN1_EL1 — Interrupt Group 1 Enable
 * ======================================================================== */
#define ICC_IGRPEN1_ENABLE  (1U << 0)   /* Group 1 enable */

static inline uint64_t icc_read_igrpen1_el1(void)
{
    return read_sysreg(ICC_IGRPEN1_EL1);
}

static inline void icc_write_igrpen1_el1(uint64_t val)
{
    write_sysreg(ICC_IGRPEN1_EL1, val);
}

/* ========================================================================
 * ICC_IAR0_EL1 — Interrupt Acknowledge Register Group 0
 * ======================================================================== */
#define ICC_IAR0_INTID_MASK     0xFFFFFF    /* Interrupt ID [23:0] */

static inline uint64_t icc_read_iar0_el1(void)
{
    return read_sysreg(ICC_IAR0_EL1);
}

/* ========================================================================
 * ICC_IAR1_EL1 — Interrupt Acknowledge Register Group 1
 * ======================================================================== */
#define ICC_IAR1_INTID_MASK     0xFFFFFF    /* Interrupt ID [23:0] */

static inline uint64_t icc_read_iar1_el1(void)
{
    return read_sysreg(ICC_IAR1_EL1);
}

/* ========================================================================
 * ICC_EOIR0_EL1 — End of Interrupt Register Group 0
 * ======================================================================== */
#define ICC_EOIR0_INTID_MASK    0xFFFFFF    /* Interrupt ID [23:0] */

static inline void icc_write_eoir0_el1(uint64_t val)
{
    write_sysreg(ICC_EOIR0_EL1, val);
}

/* ========================================================================
 * ICC_EOIR1_EL1 — End of Interrupt Register Group 1
 * ======================================================================== */
#define ICC_EOIR1_INTID_MASK    0xFFFFFF    /* Interrupt ID [23:0] */

static inline void icc_write_eoir1_el1(uint64_t val)
{
    write_sysreg(ICC_EOIR1_EL1, val);
}

/* ========================================================================
 * ICC_DIR_EL1 — Deactivate Interrupt Register
 * ======================================================================== */
#define ICC_DIR_INTID_MASK      0xFFFFFF    /* Interrupt ID [23:0] */

static inline void icc_write_dir_el1(uint64_t val)
{
    write_sysreg(ICC_DIR_EL1, val);
}

/* ========================================================================
 * ICC_RPR_EL1 — Running Priority Register (read-only)
 * ======================================================================== */
#define ICC_RPR_PRIORITY_MASK   0xFF        /* Running priority [7:0] */

static inline uint64_t icc_read_rpr_el1(void)
{
    return read_sysreg(ICC_RPR_EL1);
}

/* ========================================================================
 * ICC_HPPIR0_EL1 — Highest Priority Pending Interrupt Group 0 (read-only)
 * ======================================================================== */
#define ICC_HPPIR0_INTID_MASK   0xFFFFFF    /* Interrupt ID [23:0] */

static inline uint64_t icc_read_hppir0_el1(void)
{
    return read_sysreg(ICC_HPPIR0_EL1);
}

/* ========================================================================
 * ICC_HPPIR1_EL1 — Highest Priority Pending Interrupt Group 1 (read-only)
 * ======================================================================== */
#define ICC_HPPIR1_INTID_MASK   0xFFFFFF    /* Interrupt ID [23:0] */

static inline uint64_t icc_read_hppir1_el1(void)
{
    return read_sysreg(ICC_HPPIR1_EL1);
}

/* ========================================================================
 * ICC_AP0R0_EL1 ~ ICC_AP0R3_EL1 — Active Priorities Group 0
 * ======================================================================== */
static inline uint64_t icc_read_ap0r0_el1(void)  { return read_sysreg(ICC_AP0R0_EL1); }
static inline uint64_t icc_read_ap0r1_el1(void)  { return read_sysreg(ICC_AP0R1_EL1); }
static inline uint64_t icc_read_ap0r2_el1(void)  { return read_sysreg(ICC_AP0R2_EL1); }
static inline uint64_t icc_read_ap0r3_el1(void)  { return read_sysreg(ICC_AP0R3_EL1); }

/* ========================================================================
 * ICC_AP1R0_EL1 ~ ICC_AP1R3_EL1 — Active Priorities Group 1
 * ======================================================================== */
static inline uint64_t icc_read_ap1r0_el1(void)  { return read_sysreg(ICC_AP1R0_EL1); }
static inline uint64_t icc_read_ap1r1_el1(void)  { return read_sysreg(ICC_AP1R1_EL1); }
static inline uint64_t icc_read_ap1r2_el1(void)  { return read_sysreg(ICC_AP1R2_EL1); }
static inline uint64_t icc_read_ap1r3_el1(void)  { return read_sysreg(ICC_AP1R3_EL1); }

/* ========================================================================
 * ICC_SGI0R_EL1 — Software Generated Interrupt Group 0
 * ======================================================================== */
#define ICC_SGI0R_INTID_MASK        0xF         /* SGI INTID [3:0] */
#define ICC_SGI0R_INTID_SHIFT       24
#define ICC_SGI0R_IRM               (1ULL << 40) /* Interrupt Routing Mode: all PEs */
#define ICC_SGI0R_AFF1_SHIFT        16
#define ICC_SGI0R_AFF1_MASK         0xFF
#define ICC_SGI0R_AFF2_SHIFT        32
#define ICC_SGI0R_AFF2_MASK         0xFF
#define ICC_SGI0R_AFF3_SHIFT        48
#define ICC_SGI0R_AFF3_MASK         0xFF
#define ICC_SGI0R_TARGET_LIST_SHIFT 0
#define ICC_SGI0R_TARGET_LIST_MASK  0xFFFF

static inline void icc_write_sgi0r_el1(uint64_t val)
{
    write_sysreg(ICC_SGI0R_EL1, val);
}

/* ========================================================================
 * ICC_SGI1R_EL1 — Software Generated Interrupt Group 1
 * ======================================================================== */
#define ICC_SGI1R_INTID_MASK        0xF
#define ICC_SGI1R_INTID_SHIFT       24
#define ICC_SGI1R_IRM               (1ULL << 40)
#define ICC_SGI1R_AFF1_SHIFT        16
#define ICC_SGI1R_AFF1_MASK         0xFF
#define ICC_SGI1R_AFF2_SHIFT        32
#define ICC_SGI1R_AFF2_MASK         0xFF
#define ICC_SGI1R_AFF3_SHIFT        48
#define ICC_SGI1R_AFF3_MASK         0xFF
#define ICC_SGI1R_TARGET_LIST_SHIFT 0
#define ICC_SGI1R_TARGET_LIST_MASK  0xFFFF

static inline void icc_write_sgi1r_el1(uint64_t val)
{
    write_sysreg(ICC_SGI1R_EL1, val);
}

/* ========================================================================
 * ICC_ASGI1R_EL1 — Aliased SGI Generation Group 1
 * ======================================================================== */
#define ICC_ASGI1R_INTID_MASK       0xF
#define ICC_ASGI1R_INTID_SHIFT      24
#define ICC_ASGI1R_IRM              (1ULL << 40)
#define ICC_ASGI1R_AFF1_SHIFT       16
#define ICC_ASGI1R_AFF1_MASK        0xFF
#define ICC_ASGI1R_AFF2_SHIFT       32
#define ICC_ASGI1R_AFF2_MASK        0xFF
#define ICC_ASGI1R_AFF3_SHIFT       48
#define ICC_ASGI1R_AFF3_MASK        0xFF
#define ICC_ASGI1R_TARGET_LIST_SHIFT 0
#define ICC_ASGI1R_TARGET_LIST_MASK  0xFFFF

static inline void icc_write_asgi1r_el1(uint64_t val)
{
    write_sysreg(ICC_ASGI1R_EL1, val);
}

/* ========================================================================
 * Spurious interrupt ID returned by IAR when no interrupt is pending
 * ======================================================================== */
#define ICC_SPURIOUS_INTID  1023    /* Special INTID indicating no pending interrupt */

#endif /* __ICC_H__ */
