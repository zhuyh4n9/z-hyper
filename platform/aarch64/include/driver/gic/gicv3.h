#ifndef __GICV3_H__
#define __GICV3_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "layout.h"

#define SGI_FIRST_INTID     0
#define SGI_LAST_INTID      15

#define PPI_FIRST_INTID     16
#define PPI_LAST_INTID      31


#define SPI_FIRST_INTID     32
#define SPI_LAST_INTID      1019

#define MAX_SPI_CONTEXTS  (SPI_LAST_INTID - SPI_FIRST_INTID + 1)

typedef struct gicd_context gicd_context_t;
typedef struct gicr_context gicr_context_t;
typedef struct intr_context intr_context_t;

typedef struct gic_intr_operations {
    int (*set_intr)(intr_context_t *ctx);
    int (*clear_intr)(intr_context_t *ctx);
    int (*set_priority)(intr_context_t *ctx, uint8_t priority);
    int (*set_trigger)(intr_context_t *ctx, bool is_edge);
    int (*set_group)(intr_context_t *ctx, uint8_t group);
} gic_intr_operations_t;

typedef int (*intr_handler_t)(uint32_t intid, void *arg);

// itarget is not allowed in the driver, use either 1ofN or affinity routing instead
struct intr_context {
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
    union {
        uint32_t affinity;      //spi
        uint32_t cpu_id;        //sgi/ppi
    };
    intr_handler_t handler;
    void *arg;
    gic_intr_operations_t *ops;
};

typedef struct intr_context spi_context_t;
typedef struct intr_context sgi_context_t;
typedef struct intr_context ppi_context_t;

struct gicr_context {
    uint32_t cpu_id;
    // ctrl info
    uint32_t enable_lpi: 1;  // ignored
    uint32_t dpg0: 1; // ignored
    uint32_t dpg1s: 1;  // ignored
    uint32_t dpg1ns: 1; // ignored
    uint32_t reserved: 28;

    // identifier, read from iidr
    uint32_t implementer: 12; // Implementer code
    uint32_t revision: 4; // Revision number
    uint32_t variant: 4; // Variant number
    uint32_t productid: 8; // Product ID
    uint32_t reserved3: 4; // Reserved bits
    
    // type info
    uint64_t plpis: 1;
    uint64_t vlpis: 1;
    uint64_t direct_lpis: 1;
    uint64_t last: 1;
    uint64_t dpgs: 1;
    uint64_t processor_number: 16;
    uint64_t common_lpi_aff: 2;
    uint64_t affinity: 32;
    uint64_t reserved2: 9; // Reserved bits

    struct gicv3_redistributor_regs *gicr_regs;
    struct gicv3_redistributor_sgi_regs *gicr_sgi_regs;
    sgi_context_t sgi_ctxs[16];
    ppi_context_t ppi_ctxs[16];
};

struct gicd_context {
    uint32_t spi_count: 10; // Number of valid SPI contexts
    // gicd configs, Writable
    uint32_t enable_grp0: 1; // Enable Group 0 interrupts, ignored for this EL2 software
    uint32_t enable_grp1: 1; // Enable Group 1 interrupts, ignored for this EL2 software
    uint32_t are: 1; // Affinity Routing Enable
    // Read only
    uint32_t e1nwf: 1; // Enable 1 of N Wake-up
    uint32_t ds: 1; // Disable Security bit, ignored for this EL2 software
    uint32_t reserved: 17; // Reserved bits

    // gicd implementation info, read from typer
    uint32_t itlinesnumber: 5; // number of spi interrupts
    uint32_t cpunumber: 3;  // number of CPU interfaces
    uint32_t securityextn: 1;  // whether support security extension
    uint32_t mbis: 1;   // Message-based interrupts support
    uint32_t lpis: 1;   // LPI supports
    uint32_t num_lpis: 5; // Number of LPIs
    uint32_t a3v: 1; // Affinity level 3 valid
    uint32_t no1n: 1; // No 1-of-N support
    uint32_t reserved2: 8; // Reserved bits

    // identifier, read from iidr
    uint32_t implementer: 12; // Implementer code
    uint32_t revision: 4; // Revision number
    uint32_t variant: 4; // Variant number
    uint32_t productid: 8; // Product ID
    uint32_t reserved3: 4; // Reserved bits

    struct gicv3_distributor_regs *gicd_regs;
    struct gicr_context gicr_ctxs[MAX_SUPPORTED_CPUS];
    spi_context_t spi_ctxs[MAX_SPI_CONTEXTS];
    // future extension: add SGI context, PPI context，(LPI context), etc.
};

uint32_t gicd_status(void);
uint32_t gicd_reset_status(uint32_t status);

spi_context_t *get_spi_context(uint32_t intid);
gicd_context_t *get_gicd_context(void);
int gicd_enable_spi(uint32_t intid);

#define CHECK_SPI_INTID(intid) \
        if ((intid) < SPI_FIRST_INTID || (intid) > SPI_LAST_INTID)

int gicv3_init(void);

gicd_context_t *gicr_get(uint32_t cpu_id);

#define CHECK_SGI_INTID(intid) \
        if ((intid) > SGI_LAST_INTID)

#define CHECK_PPI_INTID(intid) \
        if ((intid) < PPI_FIRST_INTID || (intid) > PPI_LAST_INTID)

ppi_context_t *get_ppi_context(gicr_context_t *gicr_ctx, uint32_t intid);
sgi_context_t *get_sgi_context(gicr_context_t *gicr_ctx, uint32_t intid);

int gicr_sgi_enable(gicr_context_t *gicr_ctx, uint32_t intid);
int gicr_ppi_enable(gicr_context_t *gicr_ctx, uint32_t intid);

static inline bool intr_fiq(intr_context_t *ctx)
{
    return (ctx->group == 0);
}

#endif
