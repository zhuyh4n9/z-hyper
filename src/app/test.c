#include "pl011/pl011.h"
#include "aarch64_utils.h"
#include "debug.h"
#include "utils/miniheap.h"
#include "layout.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void miniheap_reliability_stress_test(void);

void test_memset(void)
{
    char buffer[20];
    memset(buffer, 'A', sizeof(buffer));
    buffer[19] = '\0';
    printf("memset test: %s\n", buffer);
}

void test_memcpy(void)
{
    char src[] = "Hello, World!";
    char dest[20];
    memcpy(dest, src, strlen(src) + 1);
    printf("memcpy test: %s\n", dest);
}

void test_rand()
{
    srand(tick_el2());
    printf("rand test: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", rand());
    }
    printf("\n");
}
