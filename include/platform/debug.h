#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__

#include <stdio.h>
#include "platform/aarch64_utils.h"

void dump_esr(uint64_t esr);

int el2_sync(struct aarch64_gpregs *regs);
void panic(const char *msg);
void aarch64_backtrace(struct aarch64_gpregs *regs, int max_frames);

#endif
