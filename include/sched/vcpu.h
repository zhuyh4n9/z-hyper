#ifndef __VCPU_H__
#define __VCPU_H__

#include "list/list.h"
#include "aarch64_utils.h"

enum vcpu_state {
    VCPU_STATE_OFFLINE,
    VCPU_STATE_RUNNING,
    VCPU_STATE_SUSPENDED,
    VCPU_STATE_READY,
    VCPU_STATE_HALTED,
};

struct gic_lrs {
    uint32_t vIntid: 10;
    uint32_t pIntid: 10;
    uint32_t res0: 3;
    uint32_t priority: 6;
    uint32_t state: 2;
    uint32_t group: 1;
    uint32_t hw: 1;
};

typedef struct sysregs_el1 {
    uint64_t elr_el1;
    uint64_t spsr_el1;
    uint64_t mpidr_el1;
    uint64_t midr_el1;
    uint64_t sp_el1;
    uint64_t sp_el0;
    uint64_t ttbr0_el1;
    uint64_t ttbr1_el1;
    uint64_t tcr_el1;
    uint64_t vbar_el1;
    uint64_t sctlr_el1;
} sysregs_el1_t;

typedef struct pcpu pcpu_t;
typedef struct vcpu {
    uint32_t id;
    enum vcpu_state state;
    struct list_head irq_list;
    struct list_head preempt_list;
    struct gic_lrs lrs[16];
    aarch64_gpregs_t gpregs;
    sysregs_el1_t sysregs_el1;
    struct list_head node;
    pcpu_t *pcpu;
} vcpu_t;

struct pcpu {
    uint32_t id;
    struct list_head ready_vcpus;
    struct list_head suspend_vcpus;
    vcpu_t *current_vcpu;
};

void vcpu_init(struct vcpu *vcpu, pcpu_t *pcpu, uint32_t id);
void vcpu_suspend(struct vcpu *vcpu);
void vcpu_resume(struct vcpu *vcpu);
void vcpu_halt(struct vcpu *vcpu);

void vcpu_start(struct vcpu *vcpu);

void pcpu_switch(pcpu_t *pcpu, aarch64_gpregs_t *gpregs);

void vcpu_save_context(struct vcpu *vcpu, const aarch64_gpregs_t *gpregs);
void vcpu_restore_context(struct vcpu *vcpu, aarch64_gpregs_t *gpregs);

#define SAVE_CPU_CONTEXT(gpregs) \
    asm volatile ( \
        "stp x0, x1, [%0, #16 * 0]\n\t" \
        "stp x2, x3, [%0, #16 * 1]\n\t" \
        "stp x4, x5, [%0, #16 * 2]\n\t" \
        "stp x6, x7, [%0, #16 * 3]\n\t" \
        "stp x8, x9, [%0, #16 * 4]\n\t" \
        "stp x10, x11, [%0, #16 * 5]\n\t" \
        "stp x12, x13, [%0, #16 * 6]\n\t" \
        "stp x14, x15, [%0, #16 * 7]\n\t" \
        "stp x16, x17, [%0, #16 * 8]\n\t" \
        "stp x18, x19, [%0, #16 * 9]\n\t" \
        "stp x20, x21, [%0, #16 * 10]\n\t" \
        "stp x22, x23, [%0, #16 * 11]\n\t" \
        "stp x24, x25, [%0, #16 * 12]\n\t" \
        "stp x26, x27, [%0, #16 * 13]\n\t" \
        "stp x28, x29, [%0, #16 * 14]\n\t" \
        "str sp , [%0,#496] \n\t"\
        "mrs x1, SPSR_EL2\n\t"\
        "str x1, [%0,#512] \n\t"\
        "adr x1, 1f\n\t"\
        "str x1, [%0,#504] \n\t"\
        "1:\n\t"\
        : : "r"(gpregs) : "memory");

static inline void restore_cpu_context(aarch64_gpregs_t *gpregs);

#endif
