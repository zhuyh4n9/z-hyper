#include "utils/console.h"
#include <string.h>
#include <errno.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

FILE *stdout = (void *)(uint64_t)0xABCD0000;

extern const console_t __start___console[];
extern const console_t __stop___console[];

const console_t *find_console(const char *name) {
    for (const console_t *c = __start___console; c < __stop___console; c++) {
        if (strcmp(c->name, name) == 0) {
            return c;
        }
    }
    return NULL;
}

const console_t *find_console_by_id(uint32_t console_id)
{
    for (const console_t *c = __start___console; c < __stop___console; c++) {
        if (c->id == console_id) {
            return c;
        }
    }
    return NULL;
}

int console_putc(const console_t *console, int c)
{
    if (console && console->ops && console->ops->putc) {
        return console->ops->putc(console, c);
    }
    return -ENODEV;
}

int console_getc(const console_t *console)
{
    if (console && console->ops && console->ops->getc) {
        return console->ops->getc(console);
    }
    return -ENODEV;
}

int console_flush(const console_t *console)
{
    if (console && console->ops && console->ops->flush) {
        return console->ops->flush(console);
    }
    return -ENODEV;
}


int console_init(const console_t *console)
{
    if (console && console->ops && console->ops->init) {
        int ret = console->ops->init(console);
        if (ret == 0) {
            ((console_t *)console)->enabled = true;
        }
        return ret;
    }
    return -ENODEV;
}

int console_putc_all(int c)
{
    int ret = 0;
    for (const console_t *console = __start___console; console < __stop___console; console++) {
        if (console->enabled && console->ops && console->ops->putc) {
            ret = console->ops->putc(console, c);
            if (ret) {
                return ret;
            }
        }
    }
    return ret;
}

int console_flush_all(void)
{
    int ret = 0;
    for (const console_t *console = __start___console; console < __stop___console; console++) {
        if (console->enabled && console->ops && console->ops->flush) {
            ret = console->ops->flush(console);
            if (ret) {
                return ret;
            }
        }
    }
    return ret;
}


int putchar(int c)
{
    console_putc_all(c);
    return c;
}

int puts(const char *s)
{
    int count = 0;
    while (*s) {
        putchar(*s++);
        count++;
    }
    return count;
}

int fflush(FILE *stream)
{
    (void)stream;
    console_flush_all();
    return 0;
}

int console_disable(const char *name)
{
    const console_t *console = find_console(name);
    if (!console) {
        return -ENOENT;
    }
    ((console_t *)console)->enabled = false;
    return 0;
}

int console_enable(const char *name)
{
    const console_t *console = find_console(name);
    if (!console) {
        return -ENOENT;
    }
    ((console_t *)console)->enabled = true;
    return 0;
}

int console_disable_by_id(uint32_t console_id)
{
    const console_t *console = find_console_by_id(console_id);
    if (!console) {
        return -ENOENT;
    }
    ((console_t *)console)->enabled = false;
    return 0;
}