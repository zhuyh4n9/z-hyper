#include <stdint.h>
#include "utils/types.h"
#include "utils/errno.h"
#include "aarch64_utils.h"
#include "gic/gicv3.h"
#include "debug.h"

#define GICV3_INTERNAL
#include "private/gicv3_internal.h"
#undef GICV3_INTERNAL


static int __gicr_sgi_set_trigger(intr_context_t *ctx, bool is_edge)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;
    uint32_t icfgr_value = 0;

    gicr_ctx = __get_this_gicr();
    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
    }

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

static int __gicr_set_clear_sgi(uint32_t intid, bool set)
{
    uint32_t reg_offset = 0;
    uint32_t bit_offset = 0;

    gicr_context_t *gicr_ctx = __get_this_gicr();
    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
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
        gicr_wait_rwp();
    }

    return 0;
}


static int __gicr_set_sgi(intr_context_t *ctx)
{
    if (!ctx) {
        panic("Invalid context for setting SGI\n");
    }
    CHECK_SGI_INTID(ctx->intid) {
        panic("Invalid SGI intid\n");
    }

    return __gicr_set_clear_sgi(ctx->intid, true);
}

static int __gicr_clear_sgi(intr_context_t *ctx)
{
    if (!ctx) {
        panic("Invalid context for clearing SGI\n");
    }
    CHECK_SGI_INTID(ctx->intid) {
        panic("Invalid SGI intid\n");
    }
    return __gicr_set_clear_sgi(ctx->intid, false);
}

int __gicr_sgi_set_priority(intr_context_t *ctx, uint8_t priority)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t reg_offset = 0;
    uint32_t byte_offset = 0;
    uint32_t priority_value = 0;

    gicr_ctx = __get_this_gicr();
    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
    }

    CHECK_SGI_INTID(ctx->intid) {
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

static int __gicr_sgi_set_group(intr_context_t *ctx, uint8_t group)
{
    gicr_context_t *gicr_ctx = NULL;
    uint32_t bit_offset = 0;
    uint32_t value = 0;


    CHECK_SGI_INTID(ctx->intid) {
        return -EINVAL;
    }

    gicr_ctx = __get_this_gicr();
    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
    }

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

static gic_intr_operations_t g_sgi_intr_ops = {
    .set_intr = __gicr_set_sgi,
    .clear_intr = __gicr_clear_sgi,
    .set_priority = __gicr_sgi_set_priority,
    .set_trigger = __gicr_sgi_set_trigger,
    .set_group = __gicr_sgi_set_group,
};

sgi_context_t *get_sgi_context(uint32_t intid)
{
    gicr_context_t *gicr_ctx = __get_this_gicr();
    sgi_context_t *sgi = NULL;

    if (!gicr_ctx) {
        panic("Failed to get GICR context\n");
    }

    CHECK_SGI_INTID(intid) {
        return NULL;
    }
    sgi = &gicr_ctx->sgi_ctxs[intid];
    sgi->ops = &g_sgi_intr_ops;

    return sgi;
}