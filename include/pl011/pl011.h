#ifndef __PL011_H__
#define __PL011_H__

#include <stdint.h>

void pl011_init(uintptr_t base_addr);

void pl011_putchar(int c);
int pl011_getchar(void);

void pl011_flush(void);
#endif
