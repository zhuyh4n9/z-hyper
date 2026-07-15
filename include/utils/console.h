#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdint.h>

typedef struct console console_t;

typedef struct console_ops {
    int (*init)(const console_t *console);
    int (*putc)(const console_t *console, int c);
    int (*getc)(const console_t *console);
    int (*flush)(const console_t *console);
} console_ops_t;

struct console {
    uint32_t id;
    uint32_t enabled: 1;
    uint32_t stage: 5;
    uint32_t flags: 12;
    uint32_t reserved: 14;
    const char *name;
    const console_ops_t *ops;
};

const console_t *find_console(const char *name);
const console_t *find_console_by_id(uint32_t console_id);

int console_init(const console_t *console);
int console_putc(const console_t *console, int c);
int console_getc(const console_t *console);
int console_flush(const console_t *console);

int console_disable(const char *name);
int console_disable_by_id(uint32_t console_id);
int console_enable(const char *name);

int console_putc_all(int c);
int console_flush_all(void);

#define CONSOLE_REGISTER(_name, _id, _ops) \
    static const console_t __console_##_name __attribute__((used, section(".data.console"))) = { \
        .id = _id, \
        .name = #_name, \
        .enabled = 0, \
        .ops = _ops, \
    }

#endif
