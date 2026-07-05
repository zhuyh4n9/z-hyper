#include "utils/utils.h"
#include "utils/zprint.h"
#include "utils/types.h"
#include "utils/errno.h"
#include "platform/aarch64_utils.h"
#include "platform/gic/gicd.h"
#include "platform/gic/gicr.h"
#include "platform/debug.h"
#include <stdint.h>
#include <stdbool.h>

extern uint8_t __gic_context_start;
extern uint8_t __gic_context_end;

#define GIC_CONTEXT_SIZE         (&__gic_context_end - &__gic_context_start)

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
    spi_context_t spi_ctxs[MAX_SPI_CONTEXTS];
    // future extension: add SGI context, PPI context，(LPI context), etc.
};

gicd_context_t *get_gicd_context(void)
{
    return (gicd_context_t *)&__gic_context_start;
}

void gicv3_show_distributor(gicd_context_t *gicd_ctx)
{
    printf("GICD: gicd_regs=0x%lx\n", (uint64_t)gicd_ctx->gicd_regs);
    printf ("GICD: itlinesnumber=%u, spi_count=%u, cpunumber=%u\n"
        "securityextn=%u, mbis=%u, lpis=%u, num_lpis=%u, a3v=%u, no1n=%u\n",
        gicd_ctx->itlinesnumber, gicd_ctx->spi_count, gicd_ctx->cpunumber,
        gicd_ctx->securityextn, gicd_ctx->mbis, gicd_ctx->lpis,
        gicd_ctx->num_lpis, gicd_ctx->a3v, gicd_ctx->no1n);
    printf("GICD: implementer=0x%x, revision=%u, variant=%u, productid=0x%x\n",
        gicd_ctx->implementer, gicd_ctx->revision, gicd_ctx->variant,
        gicd_ctx->productid);
    printf("GICD: ds=%u, enable_grp0=%u, enable_grp1=%u, are=%u\n",
        gicd_ctx->ds, gicd_ctx->enable_grp0, gicd_ctx->enable_grp1,
        gicd_ctx->are);
    printf("GICD: e1nwf=%u\n", gicd_ctx->e1nwf);
}

static int gicv3_distributor_init(void)
{
    uint32_t typer = 0;
    gicd_context_t *gicd_ctx = NULL;
    uint32_t id = 0;
    gicv3_distributor_regs_t *gicd = (gicv3_distributor_regs_t *)GICD_BASE;
    uint32_t ctrl = 0;

    gicd_ctx = get_gicd_context();
    if (gicd_ctx == NULL) {
        panic("Failed to get GICD context\n");
    }
    gicd_ctx->gicd_regs = gicd;

    // read gicd attributes
    typer = read32((paddr_t)&gicd->typer);
    gicd_ctx->itlinesnumber = (typer >> GICD_TYPER_ITLINESNUMBER_SHIFT) & GICD_TYPER_ITLINESNUMBER_MASK;
    gicd_ctx->spi_count = (gicd_ctx->itlinesnumber + 1) * 32 - SPI_FIRST_INTID;
    gicd_ctx->cpunumber = (typer >> GICD_TYPER_CPUNUMBER_SHIFT) & GICD_TYPER_CPUNUMBER_MASK;
    gicd_ctx->securityextn = (typer & GICD_TYPER_SECURITYEXTN) ? 1 : 0;
    gicd_ctx->mbis = (typer & GICD_TYPER_MBIS) ? 1 : 0;
    gicd_ctx->lpis = (typer & GICD_TYPER_LPIS) ? 1 : 0;
    gicd_ctx->num_lpis = (typer >> GICD_TYPER_NUM_LPIS_SHIFT) & GICD_TYPER_NUM_LPIS_MASK;
    gicd_ctx->a3v = (typer & GICD_TYPER_A3V) ? 1 : 0;
    gicd_ctx->no1n = (typer & GICD_TYPER_NO1N) ? 1 : 0;
    // read gicd identifier
    id = read32((paddr_t)&gicd->iidr);
    gicd_ctx->implementer = (id >> GICD_IIDR_IMPLEMENTER_SHIFT) & GICD_IIDR_IMPLEMENTER_MASK;
    gicd_ctx->revision = (id >> GICD_IIDR_REVISION_SHIFT) & GICD_IIDR_REVISION_MASK;
    gicd_ctx->variant = (id >> GICD_IIDR_VARIANT_SHIFT) & GICD_IIDR_VARIANT_MASK;
    gicd_ctx->productid = (id >> GICD_IIDR_PRODUCTID_SHIFT) & GICD_IIDR_PRODUCTID_MASK;

    // gicd configs, write to GICD_CTLR
    gicd_ctx->enable_grp0 = 0; // Enable Group 0 interrupts, ignored for this EL2 software
    gicd_ctx->enable_grp1 = 1; // Enable Group 1 interrupts, ignored for this EL2 software
    gicd_ctx->are = 0; // Affinity Routing Enable, ignored for this EL2 software
    gicd_ctx->e1nwf = 0; // Enable 1 of N Wake-up, ignored for this EL2 software
    gicd_ctx->reserved = 0; // Reserved bits

    printf("GICv3 Distributor init: 0x%lx\n", (uint64_t)&gicd->ctlr);
    write32((paddr_t)&gicd->ctlr, 0x0);

    for (uint32_t i = 0; i < gicd_ctx->itlinesnumber; i++) {
        write32((paddr_t)&gicd->igroupr[i + 1], 0xFFFFFFFF); // all interrupts are group 1 (non-secure)
        write32((paddr_t)&gicd->icenabler[i + 1], 0xFFFFFFFF); // disable all interrupts
        write32((paddr_t)&gicd->icpendr[i + 1], 0xFFFFFFFF); // clear all pending interrupts
        write32((paddr_t)&gicd->icactiver[i + 1], 0xFFFFFFFF); // clear all active interrupts
    }

    //clear sgi pending
    for (uint32_t i = 0; i < 4; i++) {
        write32((paddr_t)&gicd->cpendsgir[i], 0xFFFFFFFF);
    }

    ctrl = (gicd_ctx->enable_grp0 << GICD_CTLR_ENABLE_GRP0_SHIFT)
        | (gicd_ctx->enable_grp1 << GICD_CTLR_ENABLE_GRP1_SHIFT)
        | (gicd_ctx->are << GICD_CTLR_ARE_SHIFT);

    write32((paddr_t)&gicd->ctlr, ctrl);

    ctrl = read32((paddr_t)&gicd->ctlr); // ensure the write is completed


    gicd_ctx->ds = (ctrl & GICD_CTLR_DS) ? 1 : 0;
    gicd_ctx->enable_grp0 = (ctrl & GICD_CTLR_ENABLE_GRP0) ? 1 : 0;
    gicd_ctx->enable_grp1 = (ctrl & GICD_CTLR_ENABLE_GRP1) ? 1 : 0;
    gicd_ctx->are = (ctrl & GICD_CTLR_ARE) ? 1 : 0;
    gicd_ctx->e1nwf = (ctrl & GICD_CTLR_E1NWF) ? 1 : 0;

    printf("gicd init done!\n");
    gicv3_show_distributor(gicd_ctx);
    return 0;    
}

static int gicv3_redistributor_init(uint32_t pe_id)
{
    (void)pe_id;
    return 0;
}

int gicv3_init(void)
{
    gicv3_distributor_init();

    for (uint32_t i = 0; i < NR_CPU; i++) {
        gicv3_redistributor_init(i);
    }
    return 0;
}

spi_context_t *get_spi_context(uint32_t intid)
{
    gicd_context_t *gicd_ctx = get_gicd_context();
    spi_context_t *spi_ctx = NULL;

    if (gicd_ctx == NULL) {
        panic("Failed to get GICD context\n");
    }

    CHECK_SPI_INTID(intid) {
        return NULL;
    }
    
    spi_ctx = &gicd_ctx->spi_ctxs[intid - SPI_FIRST_INTID];

    return spi_ctx;
}

uint32_t gicd_status(void)
{
    uint32_t status = 0;
    gicd_context_t *gicd_ctx = get_gicd_context();
    if (gicd_ctx == NULL || gicd_ctx->gicd_regs == NULL) {
        panic("Failed to get GICD context\n");
    }

    status = read32((paddr_t)&gicd_ctx->gicd_regs->statusr);

    return status;
}

uint32_t gicd_reset_status(uint32_t status)
{
    gicd_context_t *gicd_ctx = get_gicd_context();
    if (gicd_ctx == NULL || gicd_ctx->gicd_regs == NULL) {
        panic("Failed to get GICD context\n");
    }

    write32((paddr_t)&gicd_ctx->gicd_regs->statusr, status & GICD_STATUS_MASK);

    return 0;
}


static int __gicd_set_clear_spi(uint32_t intid, bool set)
{
    gicd_context_t *gicd_ctx = get_gicd_context();
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;

    if (!gicd_ctx || !gicd_ctx->gicd_regs) {
        panic("Failed to get GICD context\n");
    }
    CHECK_SPI_INTID(intid) {
        return -EINVAL;
    }

    reg_offset = intid >> 5;
    bit_offset = intid & 0x1F;

    if (set) {
        write32((paddr_t)&gicd_ctx->gicd_regs->isenabler[reg_offset], (1U << bit_offset));
    } else {
        write32((paddr_t)&gicd_ctx->gicd_regs->icenabler[reg_offset], (1U << bit_offset));
    }

    return 0;
}

static int __gicd_spi_set_trigger(uint32_t intid, bool is_edge)
{
    gicd_context_t *gicd_ctx = get_gicd_context();
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;
    uint32_t cfg_value = 0;

    if (!gicd_ctx || !gicd_ctx->gicd_regs) {
        panic("Failed to get GICD context\n");
    }
    CHECK_SPI_INTID(intid) {
        return -EINVAL;
    }

    reg_offset = intid >> 4; // each ICFGR register configures 16 interrupts
    bit_offset = (intid & 0xF) << 1; // each interrupt has 2 bits in ICFGR

    cfg_value = read32((paddr_t)&gicd_ctx->gicd_regs->icfgr[reg_offset]);
    cfg_value &= ~(0x3U << bit_offset); // clear the bits for this interrupt
    cfg_value |= ((is_edge ? GICD_ICFGR_EDGE_TRIGGERED : GICD_ICFGR_LEVEL_SENSITIVE) << bit_offset);

    write32((paddr_t)&gicd_ctx->gicd_regs->icfgr[reg_offset], cfg_value);

    return 0;
}

static int __gicd_spi_config_router(uint32_t intid, bool irm, uint32_t affinity)
{
    gicd_context_t *gicd_ctx = get_gicd_context();
    uint32_t route_value = 0;
    if (!gicd_ctx || !gicd_ctx->gicd_regs) {
        panic("Failed to get GICD context\n");
    }
    CHECK_SPI_INTID(intid) {
        return -EINVAL;
    }

    if (!gicd_ctx->are && !irm) {
        return -ENOTSUP;
    }

    if (!irm) {
        uint8_t aff0 = (affinity) & GICD_IROUTER_AFF_MASK;
        uint8_t aff1 = (affinity >> 8) & GICD_IROUTER_AFF_MASK;
        uint8_t aff2 = (affinity >> 16) & GICD_IROUTER_AFF_MASK;
        uint8_t aff3 = (affinity >> 24) & GICD_IROUTER_AFF_MASK;

        route_value = (((uint64_t)aff0) << GICD_IROUTER_AFF0_SHIFT)
                | (((uint64_t)aff1) << GICD_IROUTER_AFF1_SHIFT)
                | (((uint64_t)aff2) << GICD_IROUTER_AFF2_SHIFT)
                | (((uint64_t)aff3) << GICD_IROUTER_AFF3_SHIFT);
    } else {
        route_value = ((uint64_t)1) << GICD_IROUTER_IRM_SHIFT;
    }

    write32((paddr_t)&gicd_ctx->gicd_regs->irouter[intid], route_value);

    return 0;
}

static int __gicd_spi_set_priority(uint32_t intid, uint8_t priority)
{
    gicd_context_t *gicd_ctx = get_gicd_context();
    uint32_t reg_offset = 0;
    uint32_t byte_offset = 0;
    uint32_t priority_value = 0;

    if (!gicd_ctx || !gicd_ctx->gicd_regs) {
        panic("Failed to get GICD context\n");
    }
    CHECK_SPI_INTID(intid) {
        return -EINVAL;
    }

    reg_offset = intid >> 2; // each IPRIORITYR register configures 4 interrupts
    byte_offset = intid & 0x3; // each interrupt has 1 byte in IPRIORITYR

    priority_value = read32((paddr_t)&gicd_ctx->gicd_regs->ipriorityr[reg_offset]);
    priority_value &= ~(0xFFU << (byte_offset * 8)); // clear the bits for this interrupt
    priority_value |= ((uint32_t)priority << (byte_offset * 8));

    write32((paddr_t)&gicd_ctx->gicd_regs->ipriorityr[reg_offset], priority_value);

    return 0;
}

int gicd_enable_spi(uint32_t intid)
{
    spi_context_t *spi = get_spi_context(intid);
    gicd_context_t *gicd_ctx = get_gicd_context();
    int ret = 0;

    if (!gicd_ctx) {
        panic("Failed to get GICD context\n");
    }
    if (!spi) {
        return -ENOENT;
    }

    ret = __gicd_set_clear_spi(intid, false);
    if (ret < 0) {
        return ret;
    }

    ret = __gicd_spi_set_priority(intid, spi->priority);
    if (ret < 0) {
        return ret;
    }

    ret = __gicd_spi_config_router(intid, spi->irm, spi->affinity);
    if (ret < 0) {
        return ret;
    }
    ret = __gicd_spi_set_trigger(intid, spi->trigger);
    if (ret < 0) {
        return ret;
    }

    return __gicd_set_clear_spi(intid, true);
}
