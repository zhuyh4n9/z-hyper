.section ".text"

.global hyper_vector
.global el2_exception_init

.global el2_sync_handler

.extern el2_sync

hyper_vector:
.balign 0x800
.set vector_base, hyper_vector
.org vector_base

// current EL vectors with SP_EL0
.org vector_base + 0x000
    b .
.org vector_base + 0x080
    b .
.org vector_base + 0x100
    b .
.org vector_base + 0x180
    b .

// current EL vectors with SP_ELx
.org vector_base + 0x200
    b el2_sync_handler
.org vector_base + 0x280
    b .
.org vector_base + 0x300
    b .
.org vector_base + 0x380
    b .

// current EL vectors with SP_EL1

.org vector_base + 0x400
    b .
.org vector_base + 0x480
    b .
.org vector_base + 0x500
    b .
.org vector_base + 0x580
    b .

el2_exception_init:
    ldr x0, =hyper_vector
    msr VBAR_EL2, x0
    ret

.macro intr_save_regs
    sub sp, sp, #8 * 34
    // Save X0-X30
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]
    // Save SP
    mov x0, sp
    str x0, [sp, #248]
    // Save PC
    mrs x0, ELR_EL2
    str x0, [sp, #256]
    // Save PSTATE
    mrs x0, SPSR_EL2
    str x0, [sp, #264]
    mov x0, sp
.endm

.macro intr_restore_regs
    // Restore PSTATE
    ldr x0, [sp, #264]
    msr SPSR_EL2, x0
    // Restore SP
    ldr x0, [sp, #248]
    mov sp, x0
    // Restore X30-X0
    ldr x30, [sp, #240]
    ldp x28, x29, [sp, #224]
    ldp x26, x27, [sp, #208]
    ldp x24, x25, [sp, #192]
    ldp x22, x23, [sp, #176]
    ldp x20, x21, [sp, #160]
    ldp x18, x19, [sp, #144]
    ldp x16, x17, [sp, #128]
    ldp x14, x15, [sp, #112]
    ldp x12, x13, [sp, #96]
    ldp x10, x11, [sp, #80]
    ldp x8, x9, [sp, #64]
    ldp x6, x7, [sp, #48]
    ldp x4, x5, [sp, #32]
    ldp x2, x3, [sp, #16]
    ldp x0, x1, [sp], #16
    add sp, sp, #8 * 34
.endm

el2_sync_handler:
    intr_save_regs
    b el2_sync
    intr_restore_regs
    eret
