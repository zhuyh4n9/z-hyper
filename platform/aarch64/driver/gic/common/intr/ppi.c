#include <stdint.h>
#include "utils/types.h"
#include "utils/errno.h"
#include "aarch64_utils.h"
#include "gic/gicv3.h"
#include "debug.h"
#include <string.h>

#define GICV3_INTERNAL
#include "private/gicv3_internal.h"
#undef GICV3_INTERNAL

static int __gicr_ppi_set_trigger(irq_context_t *ctx, bool is_edge)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;
    uint32_t icfgr_value = 0;
    uint32_t intid = 0;

    gicr_ctx = __get_this_gicr();
    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
    }

    CHECK_PPI_INTID(ctx->intid) {
        return -EINVAL;
    }

    intid = ctx->intid - PPI_FIRST_INTID;

    reg_offset = intid >> 4; // each ICFGR register configures 16 interrupts
    bit_offset = (intid & 0xF) << 1; // each interrupt has 2 bits in ICFGR

    icfgr_value = read32((paddr_t)&gicr_ctx->gicr_sgi_regs->icfgr1 + reg_offset);
    icfgr_value &= ~(0x3U << bit_offset); // clear the bits for this interrupt
    icfgr_value |= ((is_edge ? 1U : 0U) << bit_offset);

    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icfgr1 + reg_offset, icfgr_value);

    return 0;
}

static int __gicr_set_clear_ppi(irq_context_t *ctx, bool set)
{
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;
    gicr_context_t *gicr_ctx = __get_this_gicr();

    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
    }

    CHECK_PPI_INTID(ctx->intid) {
        return -EINVAL;
    }

    reg_offset = ctx->intid >> 5; // each ISENABLER/ICENABLER register configures 32 interrupts
    bit_offset = ctx->intid & 0x1F; // each interrupt has 1 bit in ISENABLER/ICENABLER

    printf("trace: set_clear_ppi: intid=%u, set=%d\n", ctx->intid, set);
    if (set) {
        write32((paddr_t)&gicr_ctx->gicr_sgi_regs->isenabler0 + reg_offset, (1U << bit_offset));
    } else {
        printf("trace: clear ppi: intid=%u\n", ctx->intid);
        write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icenabler0 + reg_offset, (1U << bit_offset));
        printf("trace: waiting for rwp...\n");
        gicr_wait_rwp();
    }

    return 0;
}

static int __gicr_set_ppi(irq_context_t *ctx)
{
    return __gicr_set_clear_ppi(ctx, true);
}

static int __gicr_clear_ppi(irq_context_t *ctx)
{
    return __gicr_set_clear_ppi(ctx, false);
}

static int __gicr_ppi_set_priority(irq_context_t *ctx, uint8_t priority)
{
    uint32_t reg_offset = 0;
    uint32_t byte_offset = 0;
    uint32_t priority_value = 0;
    gicr_context_t *gicr_ctx = __get_this_gicr();

    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
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

static int __gicr_ppi_set_group(irq_context_t *ctx, uint8_t group)
{
    uint32_t bit_offset = 0;
    uint32_t value = 0;
    gicr_context_t *gicr_ctx = __get_this_gicr();

    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
    }

    CHECK_PPI_INTID(ctx->intid) {
        printf("Invalid PPI intid: %u\n", ctx->intid);
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

static gic_intr_operations_t g_ppi_intr_ops = {
    .set_intr = __gicr_set_ppi,
    .clear_intr = __gicr_clear_ppi,
    .set_priority = __gicr_ppi_set_priority,
    .set_trigger = __gicr_ppi_set_trigger,
    .set_group = __gicr_ppi_set_group,
};

ppi_context_t *get_ppi_context(uint32_t intid)
{
    ppi_context_t *ppi = NULL;
    gicr_context_t *gicr_ctx = __get_this_gicr();

    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
    }

    CHECK_PPI_INTID(intid) {
        return NULL;
    }
    printf("trace: get_ppi_context: intid=%u\n", intid);

    ppi = &gicr_ctx->ppi_ctxs[intid - PPI_FIRST_INTID];
    if (ppi->intid != intid || ppi->ops != &g_ppi_intr_ops) {
        memset(ppi, 0, sizeof(ppi_context_t));
        ppi->intid = intid;
        ppi->group = 1;
        ppi->priority = 0x80;
        ppi->trigger = 0;
        ppi->ops = &g_ppi_intr_ops;
    }

    return ppi;
}