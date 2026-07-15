#ifndef __GICV3_INTERNAL_H__
#define __GICV3_INTERNAL_H__

#ifndef GICV3_INTERNAL
#error "This header is for internal use only"
#endif

#include <stdint.h>
#include <stdbool.h>

#include "gic/gicv3.h"
#include "gicr_regs.h"
#include "gicd_regs.h"
#include "gicc.h"

struct gicr_implementer_ops {
    const char *(*get_name)(void);
    uint32_t (*init)(gicr_context_t *gicr_ctx);
    uint32_t (*deinit)(gicr_context_t *gicr_ctx);
};

struct gicd_implementer_ops {
    const char *(*get_name)(void);
    uint32_t (*init)(gicd_context_t *gicd_ctx);
    uint32_t (*deinit)(gicd_context_t *gicd_ctx);
};

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
    void *implementer_ops;
    void *implementer_private_data;
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

    void *implementer_ops;
    void *implementer_private_data;

    struct gicv3_distributor_regs *gicd_regs;
    struct gicr_context gicr_ctxs[MAX_SUPPORTED_CPUS];
    spi_context_t spi_ctxs[MAX_SPI_CONTEXTS];
    // future extension: add SGI context, PPI context，(LPI context), etc.
};

extern uint8_t __gic_context_start[];
extern uint8_t __gic_context_end[];

static inline gicd_context_t *__get_gicd(void)
{
    return (gicd_context_t *)&__gic_context_start;
}

static inline gicr_context_t *__get_gicr(uint32_t cpu_id)
{
    gicd_context_t *gicd_ctx = __get_gicd();
    if (gicd_ctx == NULL) {
        panic("Failed to get GICD context\n");
    }
    if (cpu_id >= NR_CPU) {
        panic("Invalid CPU ID for GICR context\n");
    }
    return &gicd_ctx->gicr_ctxs[cpu_id];
}

static inline gicr_context_t *__get_this_gicr(void)
{
    return __get_gicr(cpu_id());
} 

// gicd internal functions
int gicd_init(void);
bool gicd_rwp(gicd_context_t *gicd_ctx);
void gicd_wait_rwp(void);

// gicr internal functions
void gicr_wait_rwp(void);
int gicr_init(gicr_context_t *gicr_ctx, uint32_t cpu_id);
bool gicr_rwp(gicr_context_t *gicr_ctx);
bool gicr_uwp(gicr_context_t *gicr_ctx);

#define GIC_CONTEXT_SIZE         (__gic_context_end - __gic_context_start)

#endif