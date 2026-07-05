#ifndef __GICD_H__
#define __GICD_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "layout.h"


/*
 * GICv3 Distributor (GICD) Register Definitions
 * Memory-mapped at GICD_BASE (0x08000000)
 * All registers are 32-bit unless noted otherwise.
 */

/* ========================================================================
 * Register Offsets
 * ======================================================================== */

typedef struct gicv3_distributor_regs {
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
} gicv3_distributor_regs_t;

/* Static assertions — validate register offsets against GICv3 spec */
_Static_assert(sizeof(gicv3_distributor_regs_t) == 0x10000,
               "GICD struct size must be 64KB");
_Static_assert(offsetof(gicv3_distributor_regs_t, ctlr)         == 0x0000, "GICD_CTLR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, typer)        == 0x0004, "GICD_TYPER offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, iidr)         == 0x0008, "GICD_IIDR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, statusr)      == 0x0010, "GICD_STATUSR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, setspi_ns)    == 0x0040, "GICD_SETSPI_NSR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, clrspi_ns)    == 0x0048, "GICD_CLRSPI_NSR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, setspi_sr)    == 0x0050, "GICD_SETSPI_SR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, clrspi_sr)    == 0x0058, "GICD_CLRSPI_SR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, igroupr)      == 0x0080, "GICD_IGROUPR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, isenabler)    == 0x0100, "GICD_ISENABLER offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, icenabler)    == 0x0180, "GICD_ICENABLER offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, ispendr)      == 0x0200, "GICD_ISPENDR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, icpendr)      == 0x0280, "GICD_ICPENDR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, isactiver)    == 0x0300, "GICD_ISACTIVER offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, icactiver)    == 0x0380, "GICD_ICACTIVER offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, ipriorityr)   == 0x0400, "GICD_IPRIORITYR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, itargetsr)    == 0x0800, "GICD_ITARGETSR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, icfgr)        == 0x0C00, "GICD_ICFGR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, igrpmodr)     == 0x0D00, "GICD_IGRPMODR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, nsacr)        == 0x0E00, "GICD_NSACR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, sgir)         == 0x0F00, "GICD_SGIR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, cpendsgir)    == 0x0F10, "GICD_CPENDSGIR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, spendsgir)    == 0x0F20, "GICD_SPENDSGIR offset");
_Static_assert(offsetof(gicv3_distributor_regs_t, irouter)      == 0x6000, "GICD_IROUTER offset");

#define GICD_REGISTER_MAP_SIZE sizeof(gicv3_distributor_regs_t)

#define GICD_CTLR_ENABLE_GRP0_SHIFT      (0)
#define GICD_CTLR_ENABLE_GRP1_SHIFT      (1)
#define GICD_CTLR_ARE_SHIFT              (4)

#define GICD_CTLR_DS_SHIFT               (6)
#define GICD_CTLR_E1NWF_SHIFT            (7)
#define GICD_CTLR_RWP_SHIFT              (31)

#define GICD_CTLR_ENABLE_GRP0      (1U << GICD_CTLR_ENABLE_GRP0_SHIFT)   /* Enable Group 0 interrupts */
#define GICD_CTLR_ENABLE_GRP1      (1U << GICD_CTLR_ENABLE_GRP1_SHIFT)   /* Enable Group 1 interrupts */
#define GICD_CTLR_ARE              (1U << GICD_CTLR_ARE_SHIFT)   /* Affinity Routing Enable*/
#define GICD_CTLR_DS               (1U << GICD_CTLR_DS_SHIFT)   /* Disable Security */
#define GICD_CTLR_E1NWF            (1U << GICD_CTLR_E1NWF_SHIFT)   /* Enable 1 of N Wake-up */
#define GICD_CTLR_RWP              (1U << GICD_CTLR_RWP_SHIFT)  /* Register Write Pending */


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

#define GICD_IIDR_IMPLEMENTER_SHIFT     0
#define GICD_IIDR_IMPLEMENTER_MASK      0xFFF
#define GICD_IIDR_REVISION_SHIFT        12
#define GICD_IIDR_REVISION_MASK         0xF
#define GICD_IIDR_VARIANT_SHIFT         16
#define GICD_IIDR_VARIANT_MASK          0xF
#define GICD_IIDR_PRODUCTID_SHIFT       24
#define GICD_IIDR_PRODUCTID_MASK        0xFF


#define GICD_SGIR_INTID_SHIFT           0
#define GICD_SGIR_INTID_MASK            0xF
#define GICD_SGIR_CPUTARGETLIST_SHIFT   16
#define GICD_SGIR_CPUTARGETLIST_MASK    0xFF
#define GICD_SGIR_NSATT                 (1U << 15)
#define GICD_SGIR_TARGETLISTFILTER_SHIFT 24
#define GICD_SGIR_TARGETLISTFILTER_MASK  0x3

#define GICD_ICFGR_LEVEL_SENSITIVE      0x0   /* Level-sensitive */
#define GICD_ICFGR_EDGE_TRIGGERED       0x2   /* Edge-triggered */

#define GICD_IROUTER_IRM_SHIFT           31
#define GICD_IROUTER_AFF0_SHIFT          0
#define GICD_IROUTER_AFF1_SHIFT          8
#define GICD_IROUTER_AFF2_SHIFT          16
#define GICD_IROUTER_AFF3_SHIFT          32

#define GICD_IROUTER_AFF_MASK            0xFF

#define SPI_FIRST_INTID     32
#define SPI_LAST_INTID      1019

#define MAX_SPI_CONTEXTS  (SPI_LAST_INTID - SPI_FIRST_INTID + 1)

typedef int (*spi_handler_t)(uint32_t intid, void *arg);

// itarget is not allowed in the driver, use either 1ofN or affinity routing instead
typedef struct spi_context {
    uint32_t intid: 10; // Interrupt ID [32, 1019]
    /**
     * IGROUPR: Interrupt Group Register
     * 0: Group 0
     * 1: Group 1
     */
    uint32_t group: 1;  // all interrupts are group 1 (non-secure) in this EL2 software, so this bit is ignored
    /**
     * IGROUPMODR: Interrupt Group Modifier Register
     * 0: Group 1 Non Secure
     * 1: Group 1 Secure
     */
    uint32_t security: 1; // all interrupts are non-secure in this EL2 software, so this bit is ignored
    /**
     * IROUTER.IRM：Interrupt Routing Mode
     *  0: for affinity
     *  1: for 1 of N
     */
    uint32_t irm: 1;
    /**
     * ICFGR: Interrupt Configuration Register
     *  0: level-sensitive
     *  1: edge-triggered
     */
    uint32_t trigger: 1;
    /**
     * IPRIORITYR: Interrupt Priority Register
     *      0x00 (highest) to 0xFF (lowest)
     */
    uint32_t priority: 8;
    /**
     * INSACR: determine how non-secure software can access the secure interrupt(G0, G1S)
     *  00: no Non-secure access is permitted
     *  01: allow non-secure:
     *        1). set Pending Status for corresponding interrupt
     *        2). set pending status via GICD_SETSPI_NSR
     *        3). some implementations may allow non-secure to set and clear active status
     *  10: allow non-secure to:
     *        1). clear/set pending status for corresponding interrupt
     *        2). set/clear pending status via GICD_SETSPI_NSR/GICD_CLIR_SPI_NSR
     *        3). clear/set active status for corresponding interrupt
     *  11: allow non-secure full access to control the interrupt
     *        1). set/clear pending/active status for corresponding interrupt
     *        2). set/clear pending status via GICD_SETSPI_NSR/GICD_CLIR_SPI_NSR
     *        3). config Route model 
     */
    uint32_t nsac: 2; // ignored for this EL2 software
    uint32_t reserved: 8; // Reserved bits
    uint32_t affinity;
    spi_handler_t handler;
    void *arg;
} spi_context_t;

typedef struct gicd_context gicd_context_t;

// match the spec

#define GICD_STATUS_READ_RES_BIT  0
#define GICD_STATUS_WRITE_RES_BIT 1
#define GICD_STATUS_READ_WO_BIT   2
#define GICD_STATUS_WRITE_RO_BIT  3
#define GICD_STATUS_MAX_CNT       4

enum GICD_STATUS {
    GICD_STATUS_READ_RES  = (1U << GICD_STATUS_READ_RES_BIT),
    GICD_STATUS_WRITE_RES = (1U << GICD_STATUS_WRITE_RES_BIT),
    GICD_STATUS_READ_WO   = (1U << GICD_STATUS_READ_WO_BIT),
    GICD_STATUS_WRITE_RO  = (1U << GICD_STATUS_WRITE_RO_BIT),
    GICD_STATUS_MASK      = (1U << (GICD_STATUS_MAX_CNT)) - 1,
};

uint32_t gicd_status(void);
uint32_t gicd_reset_status(uint32_t status);

int gicv3_init(void);

spi_context_t *get_spi_context(uint32_t intid);
gicd_context_t *get_gicd_context(void);
int gicd_enable_spi(uint32_t intid);

#define CHECK_SPI_INTID(intid) \
        if ((intid) < SPI_FIRST_INTID || (intid) > SPI_LAST_INTID)

#endif /* __GICD_H__ */
