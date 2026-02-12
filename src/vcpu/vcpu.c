#include "vcpu/vcpu.h"

void vcpu_init(pcpu_t *pcpu, struct vcpu *vcpu, uint32_t id)
{
    memset(vcpu, 0, sizeof(struct vcpu));
    vcpu->id = id;
    vcpu->state = VCPU_STATE_OFFLINE;
    vcpu->pcpu = pcpu;
    list_init(&vcpu->irq_list);
    list_init(&vcpu->preempt_list);
    list_init(&vcpu->node);
}

void vcpu_save_context(struct vcpu *vcpu, const aarch64_gpregs_t *gpregs)
{
    memcpy(&vcpu->gpregs, gpregs, sizeof(aarch64_gpregs_t));
    vcpu->sysregs_el1.elr_el1 = read_sysreg(elr_el1);
    vcpu->sysregs_el1.spsr_el1 = read_sysreg(spsr_el1);
    vcpu->sysregs_el1.mpidr_el1 = read_sysreg(mpidr_el1);
    vcpu->sysregs_el1.midr_el1 = read_sysreg(midr_el1);
    vcpu->sysregs_el1.sp_el1 = read_sysreg(sp_el1);
    vcpu->sysregs_el1.sp_el0 = read_sysreg(sp_el0);
    vcpu->sysregs_el1.ttbr0_el1 = read_sysreg(ttbr0_el1);
    vcpu->sysregs_el1.ttbr1_el1 = read_sysreg(ttbr1_el1);
    vcpu->sysregs_el1.tcr_el1 = read_sysreg(tcr_el1);
    vcpu->sysregs_el1.vbar_el1 = read_sysreg(vbar_el1);
    vcpu->sysregs_el1.sctlr_el1 = read_sysreg(sctlr_el1);
    // save lrs
    vcpu->state = VCPU_STATE_READY;
}

void vcpu_restore_context(struct vcpu *vcpu, aarch64_gpregs_t *gpregs)
{
    memcpy(gpregs, &vcpu->gpregs, sizeof(aarch64_gpregs_t));

    write_sysreg(elr_el1, vcpu->sysregs_el1.elr_el1);
    write_sysreg(spsr_el1, vcpu->sysregs_el1.spsr_el1);
    write_sysreg(mpidr_el1, vcpu->sysregs_el1.mpidr_el1);
    write_sysreg(midr_el1, vcpu->sysregs_el1.midr_el1);
    write_sysreg(sp_el1, vcpu->sysregs_el1.sp_el1);
    write_sysreg(sp_el0, vcpu->sysregs_el1.sp_el0);
    write_sysreg(ttbr0_el1, vcpu->sysregs_el1.ttbr0_el1);
    write_sysreg(ttbr1_el1, vcpu->sysregs_el1.ttbr1_el1);
    write_sysreg(tcr_el1, vcpu->sysregs_el1.tcr_el1);
    write_sysreg(vbar_el1, vcpu->sysregs_el1.vbar_el1);
    write_sysreg(sctlr_el1, vcpu->sysregs_el1.sctlr_el1);
    // restore lrs
    vcpu->state = VCPU_STATE_RUNNING;
}

void vcpu_start(struct vcpu *vcpu)
{
    if (vcpu->state != VCPU_STATE_OFFLINE) {
        return;
    }
    vcpu->state = VCPU_STATE_READY;
    // add to pcpu ready list
    list_add_tail(&vcpu->node, &vcpu->pcpu->ready_vcpus);
}

void vcpu_suspend(struct vcpu *vcpu)
{
    if (vcpu->state != VCPU_STATE_RUNNING) {
        return;
    }
    vcpu->state = VCPU_STATE_SUSPENDED;
    // move to pcpu suspend list
    list_del(&vcpu->node);
    list_add_tail(&vcpu->node, &vcpu->pcpu->suspend_vcpus);
}

void vcpu_resume(struct vcpu *vcpu)
{
    if (vcpu->state != VCPU_STATE_SUSPENDED) {
        return;
    }
    vcpu->state = VCPU_STATE_READY;
    // move to pcpu ready list
    list_del(&vcpu->node);
    list_add_tail(&vcpu->node, &vcpu->pcpu->ready_vcpus);
}

void vcpu_halt(struct vcpu *vcpu)
{
    if (vcpu->state != VCPU_STATE_RUNNING) {
        return;
    }
    vcpu->state = VCPU_STATE_HALTED;
    // remove from pcpu ready list
    list_del(&vcpu->node);
}

void pcpu_switch(pcpu_t *pcpu, aarch64_gpregs_t *gpregs)
{
    vcpu_t *current = pcpu->current_vcpu;
    if (current) {
        vcpu_save_context(current, gpregs);
        if (current->state == VCPU_STATE_RUNNING) {
            current->state = VCPU_STATE_READY;
            list_add_tail(&current->node, &pcpu->ready_vcpus);
        }
    }

    if (!list_next(&pcpu->ready_vcpus)) {
        // no ready vcpu, idle
        pcpu->current_vcpu = NULL;
        return;
    }

    vcpu_t *next = list_entry(list_next(&pcpu->ready_vcpus), vcpu_t, node);
    list_del(&next->node);
    next->state = VCPU_STATE_RUNNING;
    pcpu->current_vcpu = next;
    vcpu_restore_context(next, gpregs);
} 