#ifndef __AARCH64_UTILS_H__
#define __AARCH64_UTILS_H__

#include <stdint.h>
#include <stdio.h>
#include "atomic/barriar.h"

#include "utils/types.h"

typedef struct aarch64_gpregs {
    uint64_t x[31];
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
} aarch64_gpregs_t;

#define ESR_EC_MASK 0x3F
#define ESR_EC_SHIFT 26

#define ESR_IL_MASK 0x1F
#define ESR_IL_SHIFT 25

#define ESR_ISS_MASK 0x1FFFFFF
#define ESR_ISS_SHIFT 0

#define ESR_EC(esr) (((esr) >> ESR_EC_SHIFT) & ESR_EC_MASK)
#define ESR_IL(esr) (((esr) >> ESR_IL_SHIFT) & ESR_IL_MASK)
#define ESR_ISS(esr) (((esr) >> ESR_ISS_SHIFT) & ESR_ISS_MASK)

enum DABT_FIELD {
    DFSC_START = 0,
    DFSC_END = 5,
    WnR_START = 6,
    WnR_END = 6,
    S1PTW_START = 7,
    S1PTW_END = 7,
    CM_START = 8,
    CM_END = 8,
    EA_START = 9,
    EA_END = 9,
    FnV_START = 10,
    FnV_END = 10,
    SET_START = 11,
    SET_END = 12,
    VNCR_START = 13,
    VNCR_END = 13,
    AR_START = 14,
    AR_END = 14,
    SF_START = 15,
    SF_END = 15,
    SRT_START = 16,
    SRT_END = 18,
    SSE_START = 19,
    SSE_END = 19,
    SAS_START = 20,
    SAS_END = 21,
    ISV_START = 24,
    ISV_END = 24
};

#define GET_DABT_FIELD(value, name) (((value) >> (name##_START)) & ((1U << ((name##_END) - (name##_START) + 1)) - 1))

enum EC_Type {
    EC_UNKNOWN = 0x00,
    EC_WFI_WFE = 0x01,
    EC_CP15_MRC_MCR = 0x03,
    EC_CP15_MRRC = 0x04,
    EC_CP14_MRC_MCR = 0x05,
    EC_CP14_LDC_STC = 0x06,
    EC_AARCH32_SIMD_FP = 0x07,
    EC_AARCH64_SIMD_FP = 0x08,
    EC_ILLEGAL_STATE = 0x0E,
    EC_SVC_AARCH32 = 0x11,
    EC_SVC_AARCH64 = 0x15,
    EC_SYS_REG_TRAP_AARCH32 = 0x12,
    EC_SYS_REG_TRAP_AARCH64 = 0x16,
    EC_INSN_ABORT_LOWER_EL = 0x20,
    EC_INSN_ABORT_SAME_EL = 0x21,
    EC_DATA_ABORT_LOWER_EL = 0x24,
    EC_DATA_ABORT_SAME_EL = 0x25,
    EC_PC_ALIGNMENT_FAULT = 0x22,
    EC_SP_ALIGNMENT_FAULT = 0x26,
    EC_FP_EXC_AARCH32 = 0x28,
    EC_FP_EXC_AARCH64 = 0x2C,
    EC_SERROR_INT = 0x2F,
    EC_BREAKPOINT_AARCH32 = 0x30,
    EC_BREAKPOINT_AARCH64 = 0x34,
    EC_SOFTWARE_STEP_AARCH32 = 0x31,
    EC_SOFTWARE_STEP_AARCH64 = 0x35,
    EC_WATCHPOINT_AARCH32 = 0x32,
    EC_WATCHPOINT_AARCH64 = 0x36,
    EC_BKPT_INSN_AARCH32 = 0x38,
    EC_VECTOR_CATCH_AARCH32 = 0x3A,
    EC_BKPT_INSN_AARCH64 = 0x3C
};

static inline uint64_t read_esr_el2(void)
{
    uint64_t esr = 0;
    asm volatile ("mrs %0, ESR_EL2" : "=r" (esr));
    return esr;
}

static inline uint32_t get_current_el(void)
{
    uint64_t el;
    asm volatile ("mrs %0, CurrentEL" : "=r" (el));
    return (uint32_t)((el >> 2) & 0x3);
}

static inline void hang(void)
{
    while (1) {
        wfi();
    }
}

static inline uint32_t pstate(void)
{
    uint64_t state;
    asm volatile ("mrs %0, SPSR_EL2" : "=r" (state));
    return (uint32_t)state;
}

static inline void set_vbar_el2(uint64_t addr)
{
    asm volatile ("msr VBAR_EL2, %0" : : "r" (addr));
}

static inline uint64_t tick_el2(void)
{
    uint64_t cntpct_el0;
    asm volatile ("mrs %0, CNTVCT_EL0" : "=r" (cntpct_el0));
    return cntpct_el0;
}

static inline uint64_t read_far_el2(void)
{
    uint64_t far;
    asm volatile ("mrs %0, FAR_EL2" : "=r" (far));
    return far;
}

#define read_sysreg(reg) ({ \
    uint64_t val; \
    asm volatile ("mrs %0, " #reg : "=r" (val)); \
    val; \
})

static inline void write32(paddr_t addr, uint32_t value)
{
    *(volatile uint32_t *)addr = value;
}

static inline uint32_t read32(paddr_t addr)
{
    return *(volatile uint32_t *)addr;
}

static inline void write64(paddr_t addr, uint64_t value)
{
    *(volatile uint64_t *)addr = value;
}

static inline uint64_t read64(paddr_t addr)
{
    return *(volatile uint64_t *)addr;
}

#define write_sysreg(reg, val) do { \
    asm volatile ("msr " #reg ", %0" : : "r" (val)); \
} while (0)

static inline uint32_t cpu_id(void)
{
    uint64_t mpidr;
    asm volatile ("mrs %0, MPIDR_EL1" : "=r" (mpidr));
    return (uint32_t)(mpidr & 0xFF);
}

#define HCR_EL2_IMO   (1UL << 4)
#define HCR_EL2_FMO   (1UL << 5)

static inline int irq_enable(void)
{
    uint64_t hcr_el2 = read_sysreg(HCR_EL2);
    hcr_el2 |= HCR_EL2_IMO | HCR_EL2_FMO; // Enable IRQ routing to EL2
    write_sysreg(HCR_EL2, hcr_el2);
    asm volatile ("msr DAIFClr, #2");
    isb();
    return 0;
}

static inline int irq_disable(void)
{
    asm volatile ("msr DAIFSet, #2");
    isb();
    return 0;
}

#endif
