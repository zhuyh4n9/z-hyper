#ifndef __UTILS_H__
#define __UTILS_H__


static inline uint32_t obtain_bits_range32(uint32_t value, uint32_t start, uint32_t end)
{
    uint32_t mask = ((1U << (end - start + 1)) - 1) << start;

    return (value & mask) >> start;
}
static inline uint64_t obtain_bits_range64(uint64_t value, uint32_t start, uint32_t end)
{
    uint64_t mask = ((1ULL << (end - start + 1)) - 1) << start;

    return (value & mask) >> start;
}

#endif
