#include <stdint.h>
#include "utils/types.h"
#include "utils/errno.h"
#include "aarch64_utils.h"
#include "gic/gicv3.h"
#include "debug.h"

#define GICV3_INTERNAL
#include "include/gicv3_internal.h"
#undef GICV3_INTERNAL

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

int gicr_init(gicr_context_t *gicr_ctx, uint32_t cpu_id)
{
    uint64_t typer = 0;

    gicr_ctx->gicr_regs = (struct gicv3_redistributor_regs *)(GICR_BASE + GICR_STRIDE * cpu_id);
    gicr_ctx->gicr_sgi_regs = (struct gicv3_redistributor_sgi_regs *)(GICR_BASE + GICR_STRIDE * cpu_id + 0x10000);

    typer = read32((paddr_t)&gicr_ctx->gicr_regs->typer);
    gicr_ctx->plpis = (typer & GICR_TYPER_PLPI) ? 1 : 0;
    gicr_ctx->vlpis = (typer & GICR_TYPER_VLPI) ? 1 : 0;
    gicr_ctx->direct_lpis = (typer & GICR_TYPER_PLPI) ? 1 : 0;
    gicr_ctx->last = (typer & GICR_TYPER_LAST) ? 1 : 0;
    gicr_ctx->dpgs = (typer & GICR_TYPER_DPG0) ? 1 : 0;
    gicr_ctx->processor_number = (typer >> GICR_TYPER_PROCESSOR_NUMBER_SHIFT) & GICR_TYPER_PROCESSOR_NUMBER_MASK;
    gicr_ctx->common_lpi_aff = (typer >> GICR_TYPER_AFFINITY_VALUE_SHIFT) & GICR_TYPER_AFFINITY_VALUE_MASK;
    gicr_ctx->affinity = (typer >> GICR_TYPER_AFFINITY_VALUE_SHIFT) & GICR_TYPER_AFFINITY_VALUE_MASK;
    gicr_ctx->reserved2 = (typer >> 55) & 0x1FF;

    printf("GICR: plpis=%u, vlpis=%u, direct_lpis=%u, last=%u, dpgs=%u, processor_number=%u, common_lpi_aff=%u, affinity=0x%lx\n",
        gicr_ctx->plpis, gicr_ctx->vlpis, gicr_ctx->direct_lpis,
        gicr_ctx->last, gicr_ctx->dpgs, gicr_ctx->processor_number,
        gicr_ctx->common_lpi_aff, gicr_ctx->affinity);

    gicr_ctx->cpu_id = cpu_id;

    gicr_ctx->enable_lpi = 0;
    gicr_ctx->dpg0 = 0;
    gicr_ctx->dpg1s = 0;
    gicr_ctx->dpg1ns = 0;

    write32((paddr_t)&gicr_ctx->gicr_regs->ctlr, 0x0);
    wait_rwp();

    // clear pending and active status for all SGIs and PPIs
    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icpendr0, 0xFFFFFFFF);
    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icactiver0, 0xFFFFFFFF);

     // disable all SGIs and PPIs
    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icenabler0, 0xFFFFFFFF);
    wait_rwp();

    return 0;
}

bool gicr_uwp(gicr_context_t *gicr_ctx)
{
    uint32_t ctlr = read32((paddr_t)&gicr_ctx->gicr_regs->ctlr);
    return (ctlr & GICR_CTLR_RWP) ? true : false;
}

bool gicr_rwp(gicr_context_t *gicr_ctx)
{
    uint32_t ctlr = read32((paddr_t)&gicr_ctx->gicr_regs->ctlr);
    return (ctlr & GICR_CTLR_RWP) ? true : false;
}

static int __gicr_sgi_set_trigger(intr_context_t *ctx, bool is_edge)
{
    gicr_context_t *gicr_ctx = NULL;
    gicd_context_t *gicd_ctx = get_gicd_context();
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;
    uint32_t icfgr_value = 0;

    if (gicd_ctx == NULL) {
        panic("Failed to get GICD context\n");
    }
    if (cpu_id() >= NR_CPU) {
        panic("SGI trigger can only be set on the target CPU\n");
    }
    gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];

    CHECK_SGI_INTID(ctx->intid) {
        return -EINVAL;
    }

    reg_offset = ctx->intid >> 4; // each ICFGR register configures 16 interrupts
    bit_offset = (ctx->intid & 0xF) << 1; // each interrupt has 2 bits in ICFGR

    icfgr_value = read32((paddr_t)&gicr_ctx->gicr_sgi_regs->icfgr0 + reg_offset);
    icfgr_value &= ~(0x3U << bit_offset); // clear the bits for this interrupt
    icfgr_value |= ((is_edge ? 1U : 0U) << bit_offset);

    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icfgr0 + reg_offset, icfgr_value);

    return 0;
}

static int __gicr_set_clear_sgi(gicr_context_t *gicr_ctx, uint32_t intid, bool set)
{
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;

    if (!gicr_ctx) {
        gicd_context_t *gicd_ctx = get_gicd_context();

        if (!gicd_ctx) {
            panic("Failed to get GICD context\n");
        }
        gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];
    }

    CHECK_SGI_INTID(intid) {
        return -EINVAL;
    }

    reg_offset = intid >> 5; // each ISENABLER/ICENABLER register configures 32 interrupts
    bit_offset = intid & 0x1F; // each interrupt has 1 bit in ISENABLER/ICENABLER

    if (set) {
        write32((paddr_t)&gicr_ctx->gicr_sgi_regs->isenabler0 + reg_offset, (1U << bit_offset));
    } else {
        write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icenabler0 + reg_offset, (1U << bit_offset));
        wait_rwp();
    }

    return 0;
}


static int __gicr_set_sgi(intr_context_t *ctx)
{
    gicr_context_t *gicr_ctx = NULL;
    if (!ctx) {
        panic("Invalid context for setting SGI\n");
    }
    CHECK_SGI_INTID(ctx->intid) {
        panic("Invalid SGI intid\n");
    }
    gicr_ctx = &get_gicd_context()->gicr_ctxs[cpu_id()];

    return __gicr_set_clear_sgi(gicr_ctx, ctx->intid, true);
}

static int __gicr_clear_sgi(intr_context_t *ctx)
{
    gicr_context_t *gicr_ctx = NULL;
    if (!ctx) {
        panic("Invalid context for clearing SGI\n");
    }
    CHECK_SGI_INTID(ctx->intid) {
        panic("Invalid SGI intid\n");
    }
    gicr_ctx = &get_gicd_context()->gicr_ctxs[cpu_id()];

    return __gicr_set_clear_sgi(gicr_ctx, ctx->intid, false);
}

int __gicr_sgi_set_priority(intr_context_t *ctx, uint8_t priority)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t reg_offset = 0;
    uint32_t byte_offset = 0;
    uint32_t priority_value = 0;

    gicd_context_t *gicd_ctx = get_gicd_context();
    if (!gicd_ctx) {
        panic("Failed to get GICD context\n");
    }

    CHECK_SGI_INTID(ctx->intid) {
        return -EINVAL;
    }

    gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];


    reg_offset = ctx->intid >> 2; // each IPRIORITYR register configures 4 interrupts
    byte_offset = ctx->intid & 0x3; // each interrupt has 1 byte in IPRIORITYR

    priority_value = read32((paddr_t)&gicr_ctx->gicr_sgi_regs->ipriorityr[reg_offset]);
    priority_value &= ~(0xFFU << (byte_offset * 8)); // clear the bits for this interrupt
    priority_value |= ((uint32_t)priority << (byte_offset * 8));

    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->ipriorityr[reg_offset], priority_value);

    return 0;
}

static int __gicr_sgi_set_group(intr_context_t *ctx, uint8_t group)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t bit_offset = 0;
    uint32_t value = 0;

    gicd_context_t *gicd_ctx = get_gicd_context();
    if (!gicd_ctx) {
        panic("Failed to get GICD context\n");
    }

    CHECK_SGI_INTID(ctx->intid) {
        return -EINVAL;
    }

    gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];

    bit_offset = ctx->intid & 0x1F; // each interrupt has 1 bit in IGROUPR

    value = read32((paddr_t)&gicr_ctx->gicr_sgi_regs->igroupr0);
    if (group) {
        value |= (1U << bit_offset); // set the bit for this interrupt
    } else {
        value &= ~(1U << bit_offset); // clear the bit for this interrupt
    }
    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->igroupr0, value);

    return 0;
}

static int __gicr_ppi_set_trigger(intr_context_t *ctx, bool is_edge)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;
    uint32_t icfgr_value = 0;

    if (!gicr_ctx) {
        gicd_context_t *gicd_ctx = get_gicd_context();

        if (!gicd_ctx) {
            panic("Failed to get GICD context\n");
        }
        gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];
    }

    CHECK_PPI_INTID(ctx->intid) {
        return -EINVAL;
    }

    reg_offset = ctx->intid >> 4; // each ICFGR register configures 16 interrupts
    bit_offset = (ctx->intid & 0xF) << 1; // each interrupt has 2 bits in ICFGR

    icfgr_value = read32((paddr_t)&gicr_ctx->gicr_sgi_regs->icfgr0 + reg_offset);
    icfgr_value &= ~(0x3U << bit_offset); // clear the bits for this interrupt
    icfgr_value |= ((is_edge ? 1U : 0U) << bit_offset);

    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icfgr0 + reg_offset, icfgr_value);

    return 0;
}

static int __gicr_set_clear_ppi(intr_context_t *ctx, bool set)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;

    if (!gicr_ctx) {
        gicd_context_t *gicd_ctx = get_gicd_context();

        if (!gicd_ctx) {
            panic("Failed to get GICD context\n");
        }
        gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];
    }

    CHECK_PPI_INTID(ctx->intid) {
        return -EINVAL;
    }

    reg_offset = ctx->intid >> 5; // each ISENABLER/ICENABLER register configures 32 interrupts
    bit_offset = ctx->intid & 0x1F; // each interrupt has 1 bit in ISENABLER/ICENABLER

    if (set) {
        write32((paddr_t)&gicr_ctx->gicr_sgi_regs->isenabler0 + reg_offset, (1U << bit_offset));
    } else {
        write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icenabler0 + reg_offset, (1U << bit_offset));
        wait_rwp();
    }

    return 0;
}

static int __gicr_set_ppi(intr_context_t *ctx)
{
    return __gicr_set_clear_ppi(ctx, true);
}

static int __gicr_clear_ppi(intr_context_t *ctx)
{
    return __gicr_set_clear_ppi(ctx, false);
}

static int __gicr_ppi_set_priority(intr_context_t *ctx, uint8_t priority)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t reg_offset = 0;
    uint32_t byte_offset = 0;
    uint32_t priority_value = 0;

    if (!gicr_ctx) {
        gicd_context_t *gicd_ctx = get_gicd_context();

        if (!gicd_ctx) {
            panic("Failed to get GICD context\n");
        }
        gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];
    }

    CHECK_PPI_INTID(ctx->intid) {
        return -EINVAL;
    }

    reg_offset = ctx->intid >> 2; // each IPRIORITYR register configures 4 interrupts
    byte_offset = ctx->intid & 0x3; // each interrupt has 1 byte in IPRIORITYR

    priority_value = read32((paddr_t)&gicr_ctx->gicr_sgi_regs->ipriorityr[reg_offset]);
    priority_value &= ~(0xFFU << (byte_offset * 8)); // clear the bits for this interrupt
    priority_value |= ((uint32_t)priority << (byte_offset * 8));

    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->ipriorityr[reg_offset], priority_value);

    return 0;
}

static int __gicr_ppi_set_group(intr_context_t *ctx, uint8_t group)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t bit_offset = 0;
    uint32_t value = 0;

    if (!gicr_ctx) {
        gicd_context_t *gicd_ctx = get_gicd_context();

        if (!gicd_ctx) {
            panic("Failed to get GICD context\n");
        }
        gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];
    }

    CHECK_PPI_INTID(ctx->intid) {
        return -EINVAL;
    }

    bit_offset = ctx->intid & 0x1F;

    value = read32((paddr_t)&gicr_ctx->gicr_sgi_regs->igroupr0);
    if (group) {
        value |= (1U << bit_offset); // set the bit for this interrupt
    } else {
        value &= ~(1U << bit_offset); // clear the bit for this interrupt
    }
    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->igroupr0, value);

    return 0;
}

gic_intr_operations_t g_sgi_intr_ops = {
    .set_intr = __gicr_set_sgi,
    .clear_intr = __gicr_clear_sgi,
    .set_priority = __gicr_sgi_set_priority,
    .set_trigger = __gicr_sgi_set_trigger,
    .set_group = __gicr_sgi_set_group,
};

gic_intr_operations_t g_ppi_intr_ops = {
    .set_intr = __gicr_set_ppi,
    .clear_intr = __gicr_clear_ppi,
    .set_priority = __gicr_ppi_set_priority,
    .set_trigger = __gicr_ppi_set_trigger,
    .set_group = __gicr_ppi_set_group, // PPI group setting is not supported in this implementation
};

