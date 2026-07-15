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

void gicd_wait_rwp(void)
{
    gicd_context_t *gicd_ctx = __get_gicd();
    if (gicd_ctx == NULL || gicd_ctx->gicd_regs == NULL) {
        panic("Failed to get GICD context\n");
    }

    while (gicd_rwp(gicd_ctx)) {
        ;
    }
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
    gicd_context_t *gicd_ctx = __get_gicd();
    if (gicd_ctx == NULL || gicd_ctx->gicd_regs == NULL) {
        panic("Failed to get GICD context\n");
    }

    status = read32((paddr_t)&gicd_ctx->gicd_regs->statusr);

    return status;
}

uint32_t gicd_reset_status(uint32_t status)
{
    gicd_context_t *gicd_ctx = __get_gicd();
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

    gicd_ctx = __get_gicd();
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
    gicd_wait_rwp();

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
        gicd_wait_rwp();
    }

    printf("gicd init done!\n");
    gicv3_show_distributor(gicd_ctx);

    return 0;    
}

bool gicd_rwp(gicd_context_t *gicd_ctx)
{
    if (!gicd_ctx || !gicd_ctx->gicd_regs) {
        panic("Failed to get GICD context\n");
    }

    uint32_t status = read32((paddr_t)&gicd_ctx->gicd_regs->statusr);
    return (status & GICD_CTLR_RWP) != 0;
}
