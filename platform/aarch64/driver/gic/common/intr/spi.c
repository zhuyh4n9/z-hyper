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
#include "private/gicv3_internal.h"
#undef GICV3_INTERNAL

// spi
int __gicd_set_clear_spi(uint32_t intid, bool set)
{
    gicd_context_t *gicd_ctx = __get_gicd();
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
        gicd_wait_rwp();
    }

    return 0;
}

int __gicd_spi_set_trigger(uint32_t intid, bool is_edge)
{
    gicd_context_t *gicd_ctx = __get_gicd();
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
    gicd_context_t *gicd_ctx = __get_gicd();
    uint64_t route_value = 0;
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

    write64((paddr_t)&gicd_ctx->gicd_regs->irouter[intid], route_value);

    return 0;
}

int __gicd_spi_set_priority(uint32_t intid, uint8_t priority)
{
    gicd_context_t *gicd_ctx = __get_gicd();
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
    gicd_context_t *gicd_ctx = __get_gicd();
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

static gic_intr_operations_t g_spi_intr_ops = {
    .set_intr = __gicd_set_spi,
    .clear_intr = __gicd_clear_spi,
    .set_priority = __gicd_set_spi_priority,
    .set_trigger = __gicd_set_spi_trigger,
    .set_group = __gicd_set_spi_group,
};

spi_context_t *get_spi_context(uint32_t intid)
{
    gicd_context_t *gicd_ctx = __get_gicd();
    spi_context_t *spi_ctx = NULL;

    if (gicd_ctx == NULL) {
        panic("Failed to get GICD context\n");
    }

    CHECK_SPI_INTID(intid) {
        return NULL;
    }

    spi_ctx = &gicd_ctx->spi_ctxs[intid - SPI_FIRST_INTID];
    spi_ctx->ops = &g_spi_intr_ops;
    return spi_ctx;
}