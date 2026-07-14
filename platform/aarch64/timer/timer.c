#include "timer/timer.h"

void percpu_timer_init(uint32_t interval_ms)
{
    uint64_t cntfrq_el0 = read_sysreg(CNTFRQ_EL0);
    uint64_t cntv_tval = (cntfrq_el0 * interval_ms) / 1000;

    write_sysreg(CNTV_TVAL_EL0, cntv_tval);
    write_sysreg(CNTV_CTL_EL0, 1);
}