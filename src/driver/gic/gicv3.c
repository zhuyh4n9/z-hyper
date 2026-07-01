#include "utils/utils.h"
#include "utils/zprint.h"
#include "utils/types.h"
#include "platform/aarch64_utils.h"
#include "platform/gic/gicd.h"
#include <stdint.h>

#define SPI_FIRST_INTID     32
#define SPI_LAST_INTID      1019

struct gicv3_distributor_regs *gicd = (struct gicv3_distributor_regs *)GICD_BASE;

int gicv3_distributor_init(void)
{
    printf("GICv3 Distributor init: 0x%lx\n", (uint64_t)&gicd->ctlr);
    write32((paddr_t)&gicd->ctlr, 0x0);

    printf("gicd disabled\n");

    for (uint32_t i = 0; i < 32; i++) {
        write32((paddr_t)&gicd->igroupr[i], 0xFFFFFFFF); // all interrupts are group 1 (non-secure)
        write32((paddr_t)&gicd->icenabler[i], 0xFFFFFFFF); // disable all interrupts
        write32((paddr_t)&gicd->icpendr[i], 0xFFFFFFFF); // clear all pending interrupts
        write32((paddr_t)&gicd->icactiver[i], 0xFFFFFFFF); // clear all active interrupts
    }

    //clear sgi pending
    for (uint32_t i = 0; i < 4; i++) {
        write32((paddr_t)&gicd->cpendsgir[i], 0xFFFFFFFF);
    }

    write32((paddr_t)&gicd->ctlr,
        GICD_CTLR_ENABLE_GRP0 | GICD_CTLR_ENABLE_GRP1_NS
        | GICD_CTLR_ARE_S | GICD_CTLR_ARE_NS);

    printf("gicd init done!\n");
    return 0;    
}

int gicd_set_spi_priority(uint32_t intid, uint8_t priority)
{
    if (intid < SPI_FIRST_INTID || intid > SPI_LAST_INTID) {
        printf("Invalid interrupt ID: %u\n", intid);
        return -1;
    }

    uint32_t reg_index = intid >> 2;
    uint32_t byte_offset = intid & 0x3;

    uint32_t current_value = read32((paddr_t)&gicd->ipriorityr[reg_index]);
    current_value &= ~(0xFF << (byte_offset * 8));
    current_value |= ((uint32_t)priority << (byte_offset * 8));

    write32((paddr_t)&gicd->ipriorityr[reg_index], current_value);
    return 0;
}

uint8_t gicd_get_spi_priority(uint32_t intid)
{
    if (intid < SPI_FIRST_INTID || intid > SPI_LAST_INTID) {
        printf("Invalid interrupt ID: %u\n", intid);
        return 0xFF; // Return an invalid priority for out-of-range IDs
    }

    uint32_t reg_index = intid >> 2;
    uint32_t byte_offset = intid & 0x3;

    uint32_t current_value = read32((paddr_t)&gicd->ipriorityr[reg_index]);
    return (uint8_t)((current_value >> (byte_offset * 8)) & 0xFF);
}

uint8_t gicd_get_spi_group(uint32_t intid)
{
    if (intid < SPI_FIRST_INTID || intid > SPI_LAST_INTID) {
        printf("Invalid interrupt ID: %u\n", intid);
        return 0xFF; // Return an invalid group for out-of-range IDs
    }

    uint32_t reg_index = intid >> 5;
    uint32_t bit_offset = intid & 0x1F;

    uint32_t current_value = read32((paddr_t)&gicd->igroupr[reg_index]);
    return (uint8_t)((current_value >> bit_offset) & 0x1);
}

uint8_t gicd_get_spi_security(uint32_t intid)
{
    if (intid < SPI_FIRST_INTID || intid > SPI_LAST_INTID) {
        printf("Invalid interrupt ID: %u\n", intid);
        return -1;
    }

    uint32_t reg_index = intid >> 5;
    uint32_t bit_offset = intid & 0x1F;

    uint32_t current_value = read32((paddr_t)&gicd->isenabler[reg_index]);
    return (uint8_t)((current_value >> bit_offset) & 0x1);
}

int gicd_set_spi_affinity(uint32_t intid, uint32_t affinity)
{
    uint64_t affinity_value = 0;
    uint32_t gicd_ctrl = 0;
    uint8_t group = 0;
    uint8_t security = 0;

    if (intid < SPI_FIRST_INTID || intid > SPI_LAST_INTID) {
        printf("Invalid interrupt ID: %u\n", intid);
        return -1;
    }
    gicd_ctrl = read32(&gicd->ctlr);
    if ((gicd_ctrl & (GICD_CTLR_ARE_S | GICD_CTLR_ARE_NS)) == 0) {
        printf("Affinity routing is not enabled in GICD_CTLR\n");
        return -1;
    }

    // FIX ME
    group = gicd_get_spi_group(intid);
    security = gicd_get_spi_security(intid);
    
    if (group == 0 && security == 0) {
        printf("Interrupt ID %u is Group 0 Secure, cannot set affinity\n", intid);
        return -1;
    }

    affinity_value |= obtain_bits_range32(affinity, 0, 7);
    affinity_value |= ((uint64_t)obtain_bits_range32(affinity, 8, 15) << 8);
    affinity_value |= ((uint64_t)obtain_bits_range32(affinity, 16, 23) << 16);
    affinity_value |= ((uint64_t)obtain_bits_range32(affinity, 24, 31) << 32);

    write32((paddr_t)&gicd->irouter[intid], (uint32_t)affinity_value);

    return 0;
}


int gicd_enable_spi(uint32_t intid)
{
    if (intid < SPI_FIRST_INTID || intid > SPI_LAST_INTID) {
        printf("Invalid interrupt ID: %u\n", intid);
        return -1;
    }

    uint32_t reg_index = intid >> 5;
    uint32_t bit_index = intid & 0x1F;

    write32((paddr_t)&gicd->isenabler[reg_index], (1U << bit_index));
    return 0;
}

int gicv3_redistributor_init(uint32_t pe_id)
{
    (void)pe_id;
    return 0;
}

int gicv3_init(void)
{
    gicv3_distributor_init();

    for (uint32_t i = 0; i < NR_CPU; i++) {
        gicv3_redistributor_init(i);
    }
    return 0;
}
