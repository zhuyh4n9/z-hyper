INCLUDE "../include/layout.h"

EXTERN(_start);
ENTRY(_start);

ZHIMGE_VADDR = 0x40200000;
ZHYPER_HEAP_SIZE = 0x100000; /* 1MB heap */

PHDRS
{
    text PT_LOAD FLAGS(5); /* R_X */
    data PT_LOAD FLAGS(6); /* RW_ */
    rodata PT_LOAD FLAGS(4); /* R_ */
}

SECTIONS
{
    . = ZHIMGE_VADDR;
    .text.boot : { *(.text.boot) } :text
    . = ALIGN(0x1000);
    .text : {
        *(.text) *(.text.*)
    } :text
    . = ALIGN(0x1000);
    .rodata : {
        *(.rodata) *(.rodata.*)
    } :rodata
    . = ALIGN(0x1000);
    .data : {
        __start___console = .;
        *(.data.console) *(.data.console.*)
        __stop___console = .;
        *(.data) *(.data.*)
    } :data
    .bss : {
        __bss_start = .;
        *(.bss) *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } :data
    . = ALIGN(0x1000);
    .gic_context : {
        __gic_context_start = .;
        . += 0x10000; /* 64KB for GIC context */
        __gic_context_end = .;
    } :data
    . = ALIGN(0x1000);
    .stack : {
        __stack_start = .;
        . += 0x8000; /* 16KB stack */
        __stack_end = .;
    } :data
    . = ALIGN(0x1000);
    .heap : {
        __heap_start = .;
        . += ZHYPER_HEAP_SIZE; /* 1MB heap */
    } :data
    HIMAGE_END = .;
}
