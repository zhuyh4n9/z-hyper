#include "utils/utils.h"
#include "utils/zprint.h"
#include "utils/types.h"
#include "utils/errno.h"
#include "aarch64_utils.h"
#include "gic/gicv3.h"
#include "debug.h"
#include <stdint.h>

#define GICV3_INTERNAL
#include "private/gicv3_internal.h"
#undef GICV3_INTERNAL

#include <stdbool.h>
    
int gicv3_init(void)
{
    gicd_context_t *gicd_ctx = __get_gicd();

    if (gicd_ctx == NULL) {
        panic("Failed to get GICD context\n");
    }

    gicd_init();

    return 0;
}

int gicv3_percpu_init(void)
{
    gicr_context_t *gicr_ctx = __get_this_gicr();
    int ret = 0;

    if (gicr_ctx == NULL) {
        panic("Failed to get GICR context\n");
    }

    ret = gicr_init(gicr_ctx, gicr_ctx->cpu_id);
    if (ret < 0) {
        return ret;
    }

    // config cpu interface
    write_sysreg(ICC_PMR_EL1, 0xFF);
    // no preempt
    write_sysreg(ICC_BPR1_EL1, 0x7);
    write_sysreg(ICC_BPR0_EL1, 0x7);
    // enable group1 interrupts
    write_sysreg(ICC_IGRPEN1_EL1, 0x1);

    return 0;
}

int intr_enable(intr_context_t *intr)
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
