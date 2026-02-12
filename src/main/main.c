#include "pl011/pl011.h"
#include "platform/aarch64_utils.h"
#include "platform/debug.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void el2_exception_init(void);

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

int platform_init() {
    pl011_init(0x09000000);

    el2_exception_init();

    printf("current EL: %d\n", get_current_el());
    printf("pstate: %x\n", pstate());
    printf("Platform initialized \n");
    test_memset();
    test_memcpy();
    test_rand();
    panic("Test panic function");
    return 0;
}