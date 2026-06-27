#include "pl011/pl011.h"
#include "platform/aarch64_utils.h"
#include "platform/debug.h"
#include "utils/miniheap.h"
#include "layout.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void miniheap_reliability_stress_test(void);

