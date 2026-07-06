#ifndef __GICV3_INTERNAL_H__
#define __GICV3_INTERNAL_H__

#ifndef GICV3_INTERNAL
#error "This header is for internal use only"
#endif

#include <stdint.h>
#include <stdbool.h>

#include "gic/gicv3.h"
#include "gicr_regs.h"
#include "gicd_regs.h"

// gicd internal functions
int gicd_init(void);
bool gicd_rwp(gicd_context_t *gicd_ctx);

// gicr internal functions
int gicr_init(gicr_context_t *gicr_ctx, uint32_t cpu_id);
bool gicr_rwp(gicr_context_t *gicr_ctx);
bool gicr_uwp(gicr_context_t *gicr_ctx);

extern gic_intr_operations_t g_spi_intr_ops; 
extern gic_intr_operations_t g_sgi_intr_ops; 
extern gic_intr_operations_t g_ppi_intr_ops; 

#endif