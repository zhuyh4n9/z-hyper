#include "utils/utils.h"
#include "utils/zprint.h"
#include "utils/types.h"
#include "utils/errno.h"
#include "aarch64_utils.h"
#include "gic/gicv3.h"
#include "debug.h"
#include <stdint.h>

#define GICV3_INTERNAL
#include "include/gicv3_internal.h"
#undef GICV3_INTERNAL

#include <stdbool.h>
    
int gicv3_init(void)
{
    gicd_context_t *gicd_ctx = get_gicd_context();

    if (gicd_ctx == NULL) {
        panic("Failed to get GICD context\n");
    }

    gicd_init();

    for (uint32_t i = 0; i < NR_CPU; i++) {
        gicr_init(&gicd_ctx->gicr_ctxs[i], i);
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
    spi_ctx->ops = &g_spi_intr_ops;
    return spi_ctx;
}


ppi_context_t *get_ppi_context(gicr_context_t *gicr_ctx, uint32_t intid)
{
    ppi_context_t *ppi = NULL;
    if (!gicr_ctx) {
        gicd_context_t *gicd_ctx = get_gicd_context();

        if (!gicd_ctx) {
            panic("Failed to get GICD context\n");
        }
        gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];
    }

    CHECK_PPI_INTID(intid) {
        return NULL;
    }

    ppi = &gicr_ctx->ppi_ctxs[intid - PPI_FIRST_INTID];
    ppi->ops = &g_ppi_intr_ops;

    return ppi;
}

sgi_context_t *get_sgi_context(gicr_context_t *gicr_ctx, uint32_t intid)
{
    if (!gicr_ctx) {
        gicd_context_t *gicd_ctx = get_gicd_context();

        if (!gicd_ctx) {
            panic("Failed to get GICD context\n");
        }
        gicr_ctx = &gicd_ctx->gicr_ctxs[cpu_id()];
    }

    CHECK_SGI_INTID(intid) {
        return NULL;
    }

    return &gicr_ctx->sgi_ctxs[intid - SGI_FIRST_INTID];
}

int gicr_intr_enable(intr_context_t *intr)
{
    int ret = 0;

    if (!intr) {
        return -ENOENT;
    }

    // set to 0 ns
    ret = intr->ops->clear_intr(intr);
    if (ret < 0) {
        return ret;
    }
    ret = intr->ops->set_priority(intr, (uint8_t)intr->priority);
    if (ret < 0) {
        return ret;
    }
    ret = intr->ops->set_group(intr, (uint8_t)intr->group);
    if (ret < 0) {
        return ret;
    }
    ret = intr->ops->set_trigger(intr, intr->trigger);
    if (ret < 0) {
        return ret;
    }

    return intr->ops->set_intr(intr);
}
