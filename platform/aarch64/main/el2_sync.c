#include "aarch64_utils.h"
#include <stdio.h>
#include "debug.h"
#include "utils/utils.h"

void dump_esr(uint64_t esr)
{
    uint64_t ec = ESR_EC(esr);
    uint64_t iss = ESR_ISS(esr);

    printf("Error Syndrome (ESR): 0x%lx\n", esr);
    printf("  Exception Class (EC): 0x%x\n", ec);
    printf("  Instruction Length (IL): %s\n", (ESR_IL(esr)) ? "32 bits" : "16 bits");
    printf("  Instruction Specific Syndrome (ISS): 0x%lx\n", iss);
    if (ec == EC_DATA_ABORT_LOWER_EL || ec == EC_DATA_ABORT_SAME_EL) {
        uint32_t dfsc = GET_DABT_FIELD(iss, DFSC);
        uint32_t wnR = GET_DABT_FIELD(iss, WnR);
        uint32_t s1ptw = GET_DABT_FIELD(iss, S1PTW);
        uint32_t cm = GET_DABT_FIELD(iss, CM);
        uint32_t ea = GET_DABT_FIELD(iss, EA);

        printf("  Data Abort Details:\n");
        printf("    Data Fault Status Code (DFSC): 0x%x\n", dfsc);
        printf("    Write not Read (WnR): %s\n", wnR ? "Write" : "Read");
        printf("    Stage 1 Translation Table Walk (S1PTW): %s\n", s1ptw ? "Yes" : "No");
        printf("    Cache Maintenance (CM): %s\n", cm ? "Yes" : "No");
        printf("    External Abort (EA): %s\n", ea ? "Yes" : "No");
    }
}

void dump_registers(struct aarch64_gpregs *regs)
{
    uint64_t esr = read_esr_el2();
    dump_esr(esr);
    printf("Register dump:\n");
    printf("FAR: 0x%lx\n", read_far_el2());
    printf("SP: 0x%lx, PC: 0x%lx, Pstate: 0x%lx\n", regs->sp, regs->pc, regs->pstate);
    for (int i = 0; i < 31; i+= 4) {
        if (i + 3 == 31) {
            printf("X%-2d: 0x%016lx  X%-2d: 0x%016lx  X%-2d: 0x%016lx\n",
               i, regs->x[i],
               i + 1, regs->x[i + 1],
               i + 2, regs->x[i + 2]);
        } else {
            printf("X%-2d: 0x%016lx  X%-2d: 0x%016lx  X%-2d: 0x%016lx  X%-2d: 0x%016lx\n",
                i, regs->x[i],
                i + 1, regs->x[i + 1],
                i + 2, regs->x[i + 2],
                i + 3, regs->x[i + 3]);
        }
    }
    aarch64_backtrace(regs, 10);
}

void panic(const char *msg)
{
    if (msg)
        printf("-------------- PANIC: %s --------------\n", msg);
    else
        printf("-------------- PANIC --------------\n", msg);
    asm volatile("mov x0, #1\n\t"
        "ldr x1, [x0]"); // dereference un-aligned address to cause exception
    printf("Unreachable code after panic\n");
}

static void el2_dabt(struct aarch64_gpregs *regs)
{
    printf("EL2 Sync Exception occurred! \n");
    dump_registers(regs);
    hang();
}

int el2_sync(struct aarch64_gpregs *regs)
{

    uint64_t esr = read_esr_el2();
    uint64_t ec = ESR_EC(esr);

    switch (ec) {
    case EC_DATA_ABORT_LOWER_EL:
    case EC_DATA_ABORT_SAME_EL:
        el2_dabt(regs);
        break;
    default:
        printf("Unhandled EL2 Sync Exception! \n");
        fflush(NULL);
        dump_registers(regs);
        hang();
        break;
    }
    return 0;
}

void aarch64_backtrace(struct aarch64_gpregs *regs, int max_frames)
{
    printf("Backtrace: \n");
    uint64_t *frame_ptr = (uint64_t *)regs->x[29]; // x29 is the frame pointer (FP)
    for (int i = 0; i < max_frames; i++) {
        if (frame_ptr == NULL) {
            break;
        }
        uint64_t return_addr = *(frame_ptr + 1); // Return address is stored at FP + 8
        printf("  Frame %d: PC = 0x%lx\n", i, return_addr);
        frame_ptr = (uint64_t *)(*frame_ptr); // Next frame pointer is stored at FP
    }
}


int el2_irq(struct aarch64_gpregs *regs)
{
    printf("EL2 IRQ Exception occurred! \n");
    dump_registers(regs);
    hang();
    return 0;
}

int el2_fiq(struct aarch64_gpregs *regs)
{
    printf("EL2 FIQ Exception occurred! \n");
    dump_registers(regs);
    hang();
    return 0;
}