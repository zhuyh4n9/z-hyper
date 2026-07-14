#ifndef __GICC_H__
#define __GICC_H__

#include <stdint.h>
#include "layout.h"

enum ctlr_bit {
    COMMON_BPR = 0,
    EOI_MODE = 1,
    PRIORITY_MASK_HINT_ENABLE = 6,
    PRI_BITS = 8,
    ID_BITS = 11,
    SERROR_INTERRUPT_SUPPORT = 14,
    AFFINITY_LEVEL_3_VALID = 15,
};

#define PRI_BITS_MASK       (0x7)
#define ID_BITS_MASK        (0x7)

enum sre_bit {
    SYSTEM_REGISTER_ENABLE = 0,
    DISABLE_FIQ_BYPASS = 1,
    DISABLE_IRQ_BYPASS = 2,
    LOW_ACCESS_LEVEL_ENABLE = 3,
};

struct gicc_context {
    uint32_t seis: 1; /* SError Interrupt support, RO*/
    uint32_t a3v: 1;  /* Affinity level 3 support, RO*/
    uint32_t iDbits: 3; /* Number of implemented interrupt ID bits, RO */
    uint32_t PRIbits: 3; /* Number of implemented priority bits, RO */
    /**
     * Priority Masking Hint Enable
     *  0 for disabled use ICC_PMR as hint
     *  1 for enabled
     */
    uint32_t pmhe: 1;
    /**
     * EOImode support
     * 0: Drop priority when writing to EOIR
     * 1: require DIR to drop priority
     */
    uint32_t eoimode: 1;
    /**
     * Common Binary Point Register support
     * 0: ICC_BPR0_EL1 and ICC_BPR1_EL1 are separate
     * 1: ICC_BPR0_EL1 for both groups, ICC_BPR1_EL1 is reserved
     */
    uint32_t cbpr: 1;
};

#endif /* __GICC_H__ */
