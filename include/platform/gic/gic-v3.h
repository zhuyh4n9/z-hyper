#ifndef __GIC_V3_H__
#define __GIC_V3_H__

#include "layout.h"

#define GICD_REG(offset)        (volatile uint32_t *)(GICD_BASE + offset)
#define GICR_REG(offset)        (volatile uint32_t *)(GICR_BASE + offset)
#define GICC_REG(offset)        (volatile uint32_t *)(GICC_BASE + offset)

#endif
