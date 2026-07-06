#include "utils/utils.h"
#include "utils/zprint.h"
#include "utils/types.h"
#include "utils/errno.h"
#include "aarch64_utils.h"
#include "gic/gicv3.h"
#include "debug.h"
#include <stdint.h>
#include <stdbool.h>

#define GICV3_INTERNAL
#include "include/gicv3_internal.h"
#undef GICV3_INTERNAL

extern uint8_t __gic_context_start;
extern uint8_t __gic_context_end;

#define GIC_CONTEXT_SIZE         (&__gic_context_end - &__gic_context_start)

static inline void wait_rwp(void)
{
    gicd_context_t *gicd_ctx = get_gicd_context();
    if (gicd_ctx == NULL || gicd_ctx->gicd_regs == NULL) {
        panic("Failed to get GICD context\n");
    }

    while (gicd_rwp(gicd_ctx)) {
        ;
    }
}

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


/**  gic internal functions */

int gicd_init(void)
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

    ctrl = (gicd_ctx->enable_grp0 << GICD_CTLR_ENABLE_GRP0_SHIFT)
        | (gicd_ctx->enable_grp1 << GICD_CTLR_ENABLE_GRP1_SHIFT)
        | (gicd_ctx->are << GICD_CTLR_ARE_SHIFT);
    write32((paddr_t)&gicd->ctlr, ctrl);
    wait_rwp();

    ctrl = read32((paddr_t)&gicd->ctlr); // ensure the write is completed


    gicd_ctx->ds = (ctrl & GICD_CTLR_DS) ? 1 : 0;
    gicd_ctx->enable_grp0 = (ctrl & GICD_CTLR_ENABLE_GRP0) ? 1 : 0;
    gicd_ctx->enable_grp1 = (ctrl & GICD_CTLR_ENABLE_GRP1) ? 1 : 0;
    gicd_ctx->are = (ctrl & GICD_CTLR_ARE) ? 1 : 0;
    gicd_ctx->e1nwf = (ctrl & GICD_CTLR_E1NWF) ? 1 : 0;

    for (uint32_t i = 0; i < gicd_ctx->itlinesnumber; i++) {
        write32((paddr_t)&gicd->icpendr[i + 1], 0xFFFFFFFF); // clear all pending interrupts
        write32((paddr_t)&gicd->icactiver[i + 1], 0xFFFFFFFF); // clear all active interrupts

        write32((paddr_t)&gicd->icenabler[i + 1], 0xFFFFFFFF); // disable all interrupts
        // must wait for RWP bit to clear before writing to ICENABLER
        wait_rwp();
    }

    printf("gicd init done!\n");
    gicv3_show_distributor(gicd_ctx);

    return 0;    
}


int __gicd_set_clear_spi(uint32_t intid, bool set)
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
        wait_rwp();
    }

    return 0;
}

int __gicd_spi_set_trigger(uint32_t intid, bool is_edge)
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

int __gicd_spi_config_router(uint32_t intid, bool irm, uint32_t affinity)
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

int __gicd_spi_set_priority(uint32_t intid, uint8_t priority)
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

int __gicd_spi_set_security(uint32_t intid, bool group, bool secure)
{
    gicd_context_t *gicd_ctx = get_gicd_context();
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;
    uint32_t value = 0;

    if (!gicd_ctx || !gicd_ctx->gicd_regs) {
        panic("Failed to get GICD context\n");
    }
    CHECK_SPI_INTID(intid) {
        return -EINVAL;
    }

    if (group) {
        reg_offset = intid >> 5; // each IGROUPR register configures 32 interrupts
        bit_offset = intid & 0x1F; // each interrupt has 1 bit in IGROUPR

        value = read32((paddr_t)&gicd_ctx->gicd_regs->igroupr[reg_offset]);
        if (secure) {
            value &= ~(1U << bit_offset); // clear the bit for this interrupt
        } else {
            value |= (1U << bit_offset); // set the bit for this interrupt
        }
        write32((paddr_t)&gicd_ctx->gicd_regs->igroupr[reg_offset], value);
    } else {
        reg_offset = intid >> 5; // each IGRPMODR register configures 32 interrupts
        bit_offset = intid & 0x1F; // each interrupt has 1 bit in IGRPMODR

        value = read32((paddr_t)&gicd_ctx->gicd_regs->igrpmodr[reg_offset]);
        if (secure) {
            value &= ~(1U << bit_offset); // clear the bit for this interrupt
        } else {
            value |= (1U << bit_offset); // set the bit for this interrupt
        }
        write32((paddr_t)&gicd_ctx->gicd_regs->igrpmodr[reg_offset], value);
    }

    return 0;
}

static int __gicd_set_spi(intr_context_t *ctx)
{
    if (!ctx) {
        panic("Invalid context for setting SPI\n");
    }

    return __gicd_set_clear_spi(ctx->intid, true);
}

static int __gicd_clear_spi(intr_context_t *ctx)
{
    if (!ctx) {
        panic("Invalid context for clearing SPI\n");
    }

    return __gicd_set_clear_spi(ctx->intid, false);
}

static int __gicd_set_spi_priority(intr_context_t *ctx, uint8_t priority)
{
    if (!ctx) {
        panic("Invalid context for setting SPI priority\n");
    }

    return __gicd_spi_set_priority(ctx->intid, priority);
}

static int __gicd_set_spi_trigger(intr_context_t *ctx, bool is_edge)
{
    if (!ctx) {
        panic("Invalid context for setting SPI trigger\n");
    }

    return __gicd_spi_set_trigger(ctx->intid, is_edge);
}

static int __gicd_set_spi_group(intr_context_t *ctx, uint8_t group)
{
    if (!ctx) {
        panic("Invalid context for setting SPI group\n");
    }

    return __gicd_spi_set_security(ctx->intid, group, false);
}

gic_intr_operations_t g_spi_intr_ops = {
    .set_intr = __gicd_set_spi,
    .clear_intr = __gicd_clear_spi,
    .set_priority = __gicd_set_spi_priority,
    .set_trigger = __gicd_set_spi_trigger,
    .set_group = __gicd_set_spi_group,
};

bool gicd_rwp(gicd_context_t *gicd_ctx)
{
    if (!gicd_ctx || !gicd_ctx->gicd_regs) {
        panic("Failed to get GICD context\n");
    }

    uint32_t status = read32((paddr_t)&gicd_ctx->gicd_regs->statusr);
    return (status & GICD_CTLR_RWP) != 0;
}