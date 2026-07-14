#ifndef __GICV3_H__
#define __GICV3_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "layout.h"

#define SGI_FIRST_INTID     0
#define SGI_LAST_INTID      15

#define PPI_FIRST_INTID     16
#define PPI_LAST_INTID      31


#define SPI_FIRST_INTID     32
#define SPI_LAST_INTID      1019

#define MAX_SPI_CONTEXTS  (SPI_LAST_INTID - SPI_FIRST_INTID + 1)

typedef struct gicd_context gicd_context_t;
typedef struct gicr_context gicr_context_t;
typedef struct irq_context irq_context_t;

typedef struct gic_intr_operations {
    int (*set_intr)(irq_context_t *ctx);
    int (*clear_intr)(irq_context_t *ctx);
    int (*set_priority)(irq_context_t *ctx, uint8_t priority);
    int (*set_trigger)(irq_context_t *ctx, bool is_edge);
    int (*set_group)(irq_context_t *ctx, uint8_t group);
} gic_intr_operations_t;

typedef int (*intr_handler_t)(uint32_t intid, void *arg);

// itarget is not allowed in the driver, use either 1ofN or affinity routing instead
struct irq_context {
    uint32_t intid: 10; // Interrupt ID [32, 1019]
    /**
     * IGROUPR: Interrupt Group Register
     * 0: Group 0
     * 1: Group 1
     */
    uint32_t group: 1;  // all interrupts are group 1 (non-secure) in this EL2 software, so this bit is ignored
    /**
     * IGROUPMODR: Interrupt Group Modifier Register
     * 0: Group 1 Non Secure
     * 1: Group 1 Secure
     */
    uint32_t security: 1; // all interrupts are non-secure in this EL2 software, so this bit is ignored
    /**
     * IROUTER.IRM：Interrupt Routing Mode
     *  0: for affinity
     *  1: for 1 of N
     */
    uint32_t irm: 1;
    /**
     * ICFGR: Interrupt Configuration Register
     *  0: level-sensitive
     *  1: edge-triggered
     */
    uint32_t trigger: 1;
    /**
     * IPRIORITYR: Interrupt Priority Register
     *      0x00 (highest) to 0xFF (lowest)
     */
    uint32_t priority: 8;
    /**
     * INSACR: determine how non-secure software can access the secure interrupt(G0, G1S)
     *  00: no Non-secure access is permitted
     *  01: allow non-secure:
     *        1). set Pending Status for corresponding interrupt
     *        2). set pending status via GICD_SETSPI_NSR
     *        3). some implementations may allow non-secure to set and clear active status
     *  10: allow non-secure to:
     *        1). clear/set pending status for corresponding interrupt
     *        2). set/clear pending status via GICD_SETSPI_NSR/GICD_CLIR_SPI_NSR
     *        3). clear/set active status for corresponding interrupt
     *  11: allow non-secure full access to control the interrupt
     *        1). set/clear pending/active status for corresponding interrupt
     *        2). set/clear pending status via GICD_SETSPI_NSR/GICD_CLIR_SPI_NSR
     *        3). config Route model 
     */
    uint32_t nsac: 2; // ignored for this EL2 software
    uint32_t reserved: 8; // Reserved bits
    union {
        uint32_t affinity;      //spi
        uint32_t cpu_id;        //sgi/ppi
    };
    intr_handler_t handler;
    void *arg;
    gic_intr_operations_t *ops;
};

typedef struct irq_context spi_context_t;
typedef struct irq_context sgi_context_t;
typedef struct irq_context ppi_context_t;
typedef struct irq_context irq_context_t;
struct gicd_context;

typedef struct gicd_context gicd_context_t;

uint32_t gicd_status(void);
uint32_t gicd_reset_status(uint32_t status);

spi_context_t *get_spi_context(uint32_t intid);
ppi_context_t *get_ppi_context(uint32_t intid);
sgi_context_t *get_sgi_context(uint32_t intid);

int gicd_enable_spi(uint32_t intid);

#define CHECK_SPI_INTID(intid) \
        if ((intid) < SPI_FIRST_INTID || (intid) > SPI_LAST_INTID)

int gicv3_init(void);
int gicv3_percpu_init(void);

int gic_irq_enable(irq_context_t *intr);

#define CHECK_SGI_INTID(intid) \
        if ((intid) > SGI_LAST_INTID)

#define CHECK_PPI_INTID(intid) \
        if ((intid) < PPI_FIRST_INTID || (intid) > PPI_LAST_INTID)

static inline bool intr_fiq(irq_context_t *ctx)
{
    return (ctx->group == 0);
}

#define PPI_TIMER_INTID_EL2     26

#endif
