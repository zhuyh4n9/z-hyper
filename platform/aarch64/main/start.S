.section .text, "ax"
.global _start
.type _start, %function
.align 4


_start:
    /* Initialize stack pointer */
    ldr x0, =__stack_end
    mov sp, x0
    /* Call platform-specific initialization */
    bl platform_init
    b .
