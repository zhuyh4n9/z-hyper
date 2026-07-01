#ifndef __GICD_H__
#define __GICD_H__

#include <stddef.h>
#include <stdint.h>
#include "layout.h"

/*
 * GICv3 Distributor (GICD) Register Definitions
 * Memory-mapped at GICD_BASE (0x08000000)
 * All registers are 32-bit unless noted otherwise.
 */

/* ========================================================================
 * Register Offsets
 * ======================================================================== */

struct gicv3_distributor_regs {
    uint32_t ctlr;
    uint32_t typer;
    uint32_t iidr;
    uint32_t reserved0[1];
    uint32_t statusr;
    // 0x14 ~ 0x1F
    uint32_t reserved1[3];
    // 0x20 ~ 0x3F
    uint32_t reserved2[8];
    uint32_t setspi_ns;
    // 0x44 ~ 0x47
    uint32_t reserved3[1];
    uint32_t clrspi_ns;
    // 0x4C ~ 0x4F
    uint32_t reserved4[1];
    uint32_t setspi_sr;
    // 0x54 ~ 0x57
    uint32_t reserved5[1];
    uint32_t clrspi_sr;
    // 0x5C ~ 0x7F
    uint32_t reserved6[9];
    // 0x80 ~ 0xFF
    uint32_t igroupr[32];
    // 0x100 ~ 0x17F
    uint32_t isenabler[32];
    // 0x180 ~ 0x1FF
    uint32_t icenabler[32];
    // 0x200 ~ 0x27F
    uint32_t ispendr[32];
    // 0x280 ~ 0x2FF
    uint32_t icpendr[32];
    // 0x300 ~ 0x37F
    uint32_t isactiver[32];
    // 0x380 ~ 0x3FF
    uint32_t icactiver[32];
    // 0x400 ~ 0x4FF
    uint32_t ipriorityr[256];
    // 0x800 ~ 0xBFF
    uint32_t itargetsr[256];
    // 0xC00 ~ 0xCFF
    uint32_t icfgr[64];
    // 0xD00 ~ 0xD7F
    uint32_t igrpmodr[32];
    // 0xD80 ~ 0xDFF
    uint32_t reserved7[32];
    // 0xE00 ~ 0xEFF
    uint32_t nsacr[64];
    // 0xF00
    uint32_t sgir;
    // 0xF04 ~ 0xF0F
    uint32_t reserved8[3];
    // 0xF10 ~ 0xF1F
    uint32_t cpendsgir[4];
    // 0xF20 ~ 0xF2F
    uint32_t spendsgir[4];
    // 0xF30 ~ 0x5FFF
    uint32_t reserved9[0x1434];
    // 0x6000 ~ 0x7FDF, entries 0..31 are reserved
    uint64_t irouter[1020];
    // 0x7FE0 ~ 0xC000
    uint32_t reserved10[0x1008];
    // 0xC000 ~ 0xFFD0
    uint32_t reserved11[0xFF4];
    // 0xFFD0 ~ 0xFFFF
    uint32_t reserved12[0xC];

};

#define GICD_REGISTER_MAP_SIZE sizeof(struct gicv3_distributor_regs);


#define GICD_CTLR                0x0000   /* Distributor Control Register */
#define GICD_TYPER               0x0004   /* Interrupt Controller Type Register */
#define GICD_IIDR                0x0008   /* Distributor Implementer Identification */
#define GICD_STATUSR             0x0010   /* Error Reporting Status (GICv3) */
#define GICD_SETSPI_NS           0x0040   /* Set SPI Non-secure */
#define GICD_CLRSPI_NS           0x0048   /* Clear SPI Non-secure */
#define GICD_SETSPI_S            0x0050   /* Set SPI Secure */
#define GICD_CLRSPI_S            0x0058   /* Clear SPI Secure */
#define GICD_IGROUPR(n)          (0x0080 + ((n) * 4))       /* Interrupt Group Registers, n=0..31 */
#define GICD_ISENABLER(n)        (0x0100 + ((n) * 4))       /* Interrupt Set-Enable, n=0..31 */
#define GICD_ICENABLER(n)        (0x0180 + ((n) * 4))       /* Interrupt Clear-Enable, n=0..31 */
#define GICD_ISPENDR(n)          (0x0200 + ((n) * 4))       /* Interrupt Set-Pending, n=0..31 */
#define GICD_ICPENDR(n)          (0x0280 + ((n) * 4))       /* Interrupt Clear-Pending, n=0..31 */
#define GICD_ISACTIVER(n)        (0x0300 + ((n) * 4))       /* Interrupt Set-Active, n=0..31 */
#define GICD_ICACTIVER(n)        (0x0380 + ((n) * 4))       /* Interrupt Clear-Active, n=0..31 */
#define GICD_IPRIORITYR(n)       (0x0400 + ((n) * 4))       /* Interrupt Priority, n=0..255 (byte-accessible) */
#define GICD_ITARGETSR(n)        (0x0800 + ((n) * 4))       /* Interrupt Processor Targets, n=0..255 */
#define GICD_ICFGR(n)            (0x0C00 + ((n) * 4))       /* Interrupt Configuration, n=0..63 */
#define GICD_IGRPMODR(n)         (0x0D00 + ((n) * 4))       /* Interrupt Group Modifier, n=0..31 */
#define GICD_NSACR(n)            (0x0E00 + ((n) * 4))       /* Non-secure Access Control, n=0..63 */
#define GICD_SGIR               0x0F00   /* Software Generated Interrupt Register */
#define GICD_CPENDSGIR(n)       (0x0F10 + ((n) * 4))       /* SGI Clear-Pending, n=0..3 */
#define GICD_SPENDSGIR(n)       (0x0F20 + ((n) * 4))       /* SGI Set-Pending, n=0..3 */
#define GICD_PIDR2              0xFFE8   /* Peripheral ID 2 */
#define GICD_PIDR4              0xFFD0   /* Peripheral ID 4 (GICv3: indicates 64K page) */

_Static_assert(offsetof(struct gicv3_distributor_regs, statusr) == GICD_STATUSR,
               "GICD STATUSR offset mismatch");
_Static_assert(offsetof(struct gicv3_distributor_regs, igroupr) == GICD_IGROUPR(0),
               "GICD IGROUPR offset mismatch");
_Static_assert(offsetof(struct gicv3_distributor_regs, nsacr) == GICD_NSACR(0),
               "GICD NSACR offset mismatch");
_Static_assert(offsetof(struct gicv3_distributor_regs, sgir) == GICD_SGIR,
               "GICD SGIR offset mismatch");
_Static_assert(offsetof(struct gicv3_distributor_regs, cpendsgir) == GICD_CPENDSGIR(0),
               "GICD CPENDSGIR offset mismatch");
_Static_assert(offsetof(struct gicv3_distributor_regs, spendsgir) == GICD_SPENDSGIR(0),
               "GICD SPENDSGIR offset mismatch");
_Static_assert(offsetof(struct gicv3_distributor_regs, irouter) == 0x6000,
               "GICD IROUTER offset mismatch");
_Static_assert(sizeof(struct gicv3_distributor_regs) == 0x10000,
               "GICD register map size mismatch");

/* ========================================================================
 * GICD_CTLR bit definitions
 * ======================================================================== */
#define GICD_CTLR_ENABLE_GRP0      (1U << 0)   /* Enable Group 0 interrupts */
#define GICD_CTLR_ENABLE_GRP1_NS   (1U << 1)   /* Enable Non-secure Group 1 */
#define GICD_CTLR_ENABLE_GRP1_S    (1U << 2)   /* Enable Secure Group 1 */
#define GICD_CTLR_ARE_S            (1U << 4)   /* Affinity Routing Enable (Secure) */
#define GICD_CTLR_ARE_NS           (1U << 5)   /* Affinity Routing Enable (Non-secure) */
#define GICD_CTLR_DS               (1U << 6)   /* Disable Security */
#define GICD_CTLR_E1NWF            (1U << 7)   /* Enable 1 of N Wake-up */
#define GICD_CTLR_RWP              (1U << 31)  /* Register Write Pending */

/* ========================================================================
 * GICD_TYPER bit definitions
 * ======================================================================== */
#define GICD_TYPER_ITLINESNUMBER_SHIFT  0
#define GICD_TYPER_ITLINESNUMBER_MASK   0x1F
#define GICD_TYPER_CPUNUMBER_SHIFT      5
#define GICD_TYPER_CPUNUMBER_MASK       0x7
#define GICD_TYPER_SECURITYEXTN         (1U << 10)
#define GICD_TYPER_LPIS                 (1U << 17)
#define GICD_TYPER_MBIS                 (1U << 16)
#define GICD_TYPER_ESPI                 (1U << 8)
#define GICD_TYPER_NUM_LPIS_SHIFT       11
#define GICD_TYPER_NUM_LPIS_MASK        0x1F
#define GICD_TYPER_IDBITS_SHIFT         19
#define GICD_TYPER_IDBITS_MASK          0x1F
#define GICD_TYPER_A3V                  (1U << 11)   /* Affinity level 3 valid (alias) */
#define GICD_TYPER_NO1N                 (1U << 25)   /* No 1-of-N support */

/* ========================================================================
 * GICD_IIDR fields
 * ======================================================================== */
#define GICD_IIDR_IMPLEMENTER_SHIFT     0
#define GICD_IIDR_IMPLEMENTER_MASK      0xFFF
#define GICD_IIDR_REVISION_SHIFT        12
#define GICD_IIDR_REVISION_MASK         0xF
#define GICD_IIDR_PRODUCTID_SHIFT       24
#define GICD_IIDR_PRODUCTID_MASK        0xFF

/* ========================================================================
 * GICD_SGIR bit definitions
 * ======================================================================== */
#define GICD_SGIR_INTID_SHIFT           0
#define GICD_SGIR_INTID_MASK            0xF
#define GICD_SGIR_CPUTARGETLIST_SHIFT   16
#define GICD_SGIR_CPUTARGETLIST_MASK    0xFF
#define GICD_SGIR_NSATT                 (1U << 15)
#define GICD_SGIR_TARGETLISTFILTER_SHIFT 24
#define GICD_SGIR_TARGETLISTFILTER_MASK  0x3

/* ========================================================================
 * GICD_ICFGR bit definitions (2 bits per interrupt, [1:0] for INTID n)
 * ======================================================================== */
#define GICD_ICFGR_LEVEL_SENSITIVE      0x0   /* Level-sensitive */
#define GICD_ICFGR_EDGE_TRIGGERED       0x2   /* Edge-triggered */

int gicv3_init(void);

#endif /* __GICD_H__ */
