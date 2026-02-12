#include "layout.h"

EXTERN(_start);
ENTRY(_start);

ZHIMGE_VADDR = 0x40200000;
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
    .stack : {
        . = ALIGN(0x1000);
        __stack_start = .;
        . += 0x4000; /* 16KB stack */
        __stack_end = .;
    }
    . = ALIGN(0x1000);
    HIMAGE_END = .;
}
