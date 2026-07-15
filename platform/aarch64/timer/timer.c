#include "timer/timer.h"
#include "gic/gicv3.h"
#include <string.h>

static void reset_percpu_timer(void)
{
    write_sysreg(CNTHP_CTL_EL2, 0);
    write_sysreg(CNTHP_TVAL_EL2, 0);
    asm volatile ("isb");
}

static void set_percpu_timer(uint32_t interval_ms)
{
    uint64_t cntfrq_el0 = read_sysreg(CNTFRQ_EL0);
    uint64_t cntv_tval = (cntfrq_el0 / 1000) * interval_ms;

    write_sysreg(CNTHP_TVAL_EL2, cntv_tval);
    write_sysreg(CNTHP_CTL_EL2, 1);
    asm volatile ("isb");
}

static int timer_irq_handler(irq_context_t *irq, struct aarch64_gpregs *regs)
{
    (void)irq;
    (void)regs;
    // Clear the timer interrupt
    reset_percpu_timer();

    // Call the user-defined callback if it exists
    printf("timer tick!\n");

    set_percpu_timer(1000);
    return 0;
}

void percpu_timer_init(uint32_t interval_ms)
{
    ppi_context_t *timer_ppi = get_ppi_context(PPI_TIMER_INTID_EL2);
    uint64_t cntfrq_el0 = read_sysreg(CNTFRQ_EL0);
    uint64_t cntv_tval = (cntfrq_el0 / 1000) * interval_ms;

    printf("frequency: %lu Hz, interval: %u ms, cntv_tval: %lu\n", cntfrq_el0, interval_ms, cntv_tval);

    timer_ppi->group = 1;
    timer_ppi->priority = 0x80;
    timer_ppi->trigger = 1;
    timer_ppi->handle = timer_irq_handler;
    printf("Setting up PPI context for timer interrupt (INTID: %u)\n", PPI_TIMER_INTID_EL2);

    gic_irq_enable((irq_context_t *)timer_ppi);

    write_sysreg(CNTHP_TVAL_EL2, cntv_tval);
    write_sysreg(CNTHP_CTL_EL2, 1);
    asm volatile ("isb");
}
