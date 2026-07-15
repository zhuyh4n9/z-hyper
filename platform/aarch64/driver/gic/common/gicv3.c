#include <stdint.h>

#include <utils/utils.h>
#include <utils/types.h>
#include <utils/errno.h>
#include <aarch64_utils.h>
#include <gic/gicv3.h>
#include <debug.h>
#include <atomic/atomic.h>
#include <spinlock/spinlock.h>

#define GICV3_INTERNAL
#include "private/gicv3_internal.h"
#undef GICV3_INTERNAL

#include <stdbool.h>
    
static int gicv3_init(void)
{
    gicd_context_t *gicd_ctx = __get_gicd();

    if (gicd_ctx == NULL) {
        panic("Failed to get GICD context\n");
    }
    irq_enable();
    gicd_init();

    return 0;
}

int plat_gicv3_init(void)
{
    static once_flag_t gicd_init_once = ONCE_INITIALIZER;
    gicr_context_t *gicr_ctx = NULL;
    uint32_t ctlr = 0;
    uint64_t sre = 0;
    int ret = 0;

    ret = run_once(&gicd_init_once, gicv3_init);

    if (ret < 0) {
        panic("GICD initialization failed\n");
    }

    gicr_ctx = __get_this_gicr();

    if (gicr_ctx == NULL) {
        panic("Failed to get GICR context\n");
    }

    ret = gicr_init(gicr_ctx, gicr_ctx->cpu_id);
    if (ret < 0) {
        return ret;
    }

    /* Enable the system register interface before programming ICC_* state. */
    sre = (1U << SYSTEM_REGISTER_ENABLE) |
          (1U << DISABLE_FIQ_BYPASS) |
          (1U << DISABLE_IRQ_BYPASS) |
          (1U << LOW_ACCESS_LEVEL_ENABLE);
    write_sysreg(ICC_SRE_EL2, sre);
    asm volatile ("isb");

    // no preemption for group 0/1
    write_sysreg(ICC_BPR1_EL1, 0x7);
    write_sysreg(ICC_PMR_EL1, 0xFF);
    // EOImode = 0, common BPR = 0, PMHE = 0
    ctlr = read_sysreg(ICC_CTLR_EL1);
    ctlr &= ~((1U << EOI_MODE) |
              (1U << COMMON_BPR) |
              (1U << PRIORITY_MASK_HINT_ENABLE));
    write_sysreg(ICC_CTLR_EL1, ctlr);
    // enable group 1 interrupts
    write_sysreg(ICC_IGRPEN1_EL1, 0x1);

    return 0;
}

int gic_irq_enable(irq_context_t *intr)
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

irq_context_t *get_irq_context(uint32_t intid)
{
    if (intid <= SGI_LAST_INTID) {
        return get_sgi_context(intid);
    } else if (intid <= PPI_LAST_INTID) {
        return get_ppi_context(intid);
    } else if (intid <= SPI_LAST_INTID) {
        return get_spi_context(intid);
    }

    printf("Invalid intid: %u\n", intid);

    return NULL;
}