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
    uint32_t ctlr = 0;

    if (gicr_ctx == NULL) {
        panic("Failed to get GICR context\n");
    }

    ret = gicr_init(gicr_ctx, gicr_ctx->cpu_id);
    if (ret < 0) {
        return ret;
    }

    // no preemption for group 0/1
    write_sysreg(ICC_BPR0_EL1, 0x7);

    // EOImode = 0, common BPR = 0, PMHE = 0
    ctlr = read_sysreg(ICC_CTLR_EL1);
    ctlr &= ~((1U << EOI_MODE) |
              (1U << COMMON_BPR) |
              (1U << PRIORITY_MASK_HINT_ENABLE));
    write_sysreg(ICC_CTLR_EL1, ctlr);
    /**
     * use system register for icc
     * disable bypass for fiq/irq
     * enable low access level
     * enable system register access
     */
    write_sysreg(ICC_SRE_EL2, (1U << SYSTEM_REGISTER_ENABLE) |
                              (1U << DISABLE_FIQ_BYPASS) |
                              (1U << DISABLE_IRQ_BYPASS) |
                              (1U << LOW_ACCESS_LEVEL_ENABLE));
    // enable group 0 and group 1 interrupts
    write_sysreg(ICC_IGRPEN1_EL1, 0x1);
    write_sysreg(ICC_IGRPEN0_EL1, 0x1);

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

