#include <stdint.h>
#include "utils/types.h"
#include "utils/errno.h"
#include "aarch64_utils.h"
#include "gic/gicv3.h"
#include "debug.h"

#define GICV3_INTERNAL
#include "private/gicv3_internal.h"
#undef GICV3_INTERNAL

void gicr_wait_rwp(void)
{
    gicr_context_t *gicr_ctx = __get_this_gicr();
    if (gicr_ctx == NULL || gicr_ctx->gicr_regs == NULL) {
        panic("Failed to get GICR context\n");
    }

    while (gicr_rwp(gicr_ctx)) {
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
    gicr_wait_rwp();

    // clear pending and active status for all SGIs and PPIs
    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icpendr0, 0xFFFFFFFF);
    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icactiver0, 0xFFFFFFFF);

     // disable all SGIs and PPIs
    write32((paddr_t)&gicr_ctx->gicr_sgi_regs->icenabler0, 0xFFFFFFFF);
    gicr_wait_rwp();

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
