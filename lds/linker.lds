INCLUDE "../include/layout.h"

EXTERN(_start);
ENTRY(_start);

ZHIMGE_VADDR = 0x40200000;
ZHYPER_HEAP_SIZE = 0x100000; /* 1MB heap */
SECTIONS
{
    . = ZHIMGE_VADDR;
    .text.boot : { *(.text.boot) }
    . = ALIGN(0x1000);
    .text : {
        *(.text) *(.text.*)
    }
    .rodata : {
        *(.rodata) *(.rodata.*)
    }
    .data : {
        *(.data) *(.data.*)
    }
    . = ALIGN(0x1000);
    .bss : {
        __bss_start = .;
        *(.bss) *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }
    . = ALIGN(0x1000);
    .gic_context : {
        . = ALIGN(0x1000);
        __gic_context_start = .;
        . += 0x10000; /* 64KB for GIC context */
        __gic_context_end = .;
    }
    . = ALIGN(0x1000);
    .stack : {
        . = ALIGN(0x1000);
        __stack_start = .;
        . += 0x8000; /* 16KB stack */
        __stack_end = .;
    }
    . = ALIGN(0x1000);
    .heap : {
        __heap_start = .;
        . += ZHYPER_HEAP_SIZE; /* 1MB heap */
    }
    HIMAGE_END = .;
}
