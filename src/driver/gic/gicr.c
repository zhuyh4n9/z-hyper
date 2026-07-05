#include <stdint.h>
#include "platform/gic/gicd.h"
#include "platform/gic/gicr.h"

struct gicr_context {
    uint32_t cpu_id;
    // ctrl info
    uint32_t enable_lpi: 1;
    uint32_t dpg0: 1; // ignored
    uint32_t dpg1s: 1;  // ignored
    uint32_t dpg1ns: 1; // ignored
    uint32_t reserved: 28;

    // identifier, read from iidr
    uint32_t implementer: 12; // Implementer code
    uint32_t revision: 4; // Revision number
    uint32_t variant: 4; // Variant number
    uint32_t productid: 8; // Product ID
    uint32_t reserved3: 4; // Reserved bits
    
        // type info
    uint64_t plpis: 1;
    uint64_t vlpis: 1;
    uint64_t direct_lpis: 1;
    uint64_t last: 1;
    uint64_t dpgs: 1;
    uint64_t processor_number: 16;
    uint64_t common_lpi_aff: 2;
    uint64_t affinity: 32;
    uint64_t reserved2: 9; // Reserved bits

    struct gicv3_redistributor_regs *gicr_regs;
    struct gicv3_redistributor_sgi_regs *gicr_sgi_regs;
};

