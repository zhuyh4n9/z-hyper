#include <stdlib.h>
#include <stdio.h>

static unsigned long next = 0xDEADBEEF;

int rand(void)
{
    next = (next * 0x1234 + 12345) * next;
    return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed)
{
    next = seed;
}

