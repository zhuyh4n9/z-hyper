#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "debug.h"
#include "pl011/pl011.h"

#define va_list         __builtin_va_list
#define va_start(v, l)  __builtin_va_start(v, l)
#define va_arg(v, l)    __builtin_va_arg(v, l)
#define va_end(v)       __builtin_va_end(v)
#define va_copy(d, s)   __builtin_va_copy(d, s)

enum printopt {
    PRINT_0X      = 1 << 0,
    ZERO_PADDING  = 1 << 1,
};

int putchar(int c)
{
    // Implement your character output function here.
    // For example, write to a UART or console.
    // This is a placeholder implementation.
    pl011_putchar(c);
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

static void print64(int64_t num, int base, bool sign, int digit, enum printopt opt)
{
    char buf[sizeof(num) * 8 + 8] = {0};
    char *end = buf + sizeof(buf);
    char *cur = end - 1;
    uint64_t unum;
    bool neg = false;

    if(sign && num < 0) {
        unum = (uint64_t)(-(num + 1)) + 1;
        neg = true;
    } else {
        unum = (uint64_t)num;
    }

    do {
        *--cur = "0123456789abcdef"[unum % base];
    } while(unum /= base);

    if(opt & PRINT_0X) {
        *--cur = 'x';
        *--cur = '0';
    }

    if(neg)
        *--cur = '-';

    int len = strlen(cur);
    if(digit > 0) {
        while(digit-- > len)
        if (opt & ZERO_PADDING)
            putchar('0');
        else
            putchar(' ');
    }
    puts(cur);
    if(digit < 0) {
        digit = -digit;
        while(digit-- > len)
            putchar(' ');
    }
}

int isdigit(int c)
{
    return '0' <= c && c <= '9';
}

static const char *fetch_digit(const char *fmt, int *digit, bool *zero_padding) {
    int n = 0, neg = 0;

    if(*fmt == '-') {
        fmt++;
        neg = 1;
    }
    *zero_padding = (*fmt == '0');
    if (*zero_padding) {
        fmt++;
    }
    while(isdigit(*fmt)) {
        n = n * 10 + *fmt++ - '0';
    }

    *digit = neg? -n : n;
    return fmt;
}

 int vprintf(const char *fmt, va_list ap)
{
    char *s;
    void *p;
    int digit = 0;

    (void)p;
    for(; *fmt; fmt++) {
        char c = *fmt;
        if(c == '%') {
            fmt++;
            bool zero_padding = false;
            fmt = fetch_digit(fmt, &digit, &zero_padding);
            switch(c = *fmt) {
            case 'd':
                print64(va_arg(ap, int32_t), 10, true, digit, zero_padding ? ZERO_PADDING : 0);
                break;
            case 'u':
                print64(va_arg(ap, uint32_t), 10, false, digit, zero_padding ? ZERO_PADDING : 0);
                break;
            case 'x':
                print64(va_arg(ap, uint64_t), 16, false, digit, zero_padding ? ZERO_PADDING : 0);
                break;
            case 'p':
                p = va_arg(ap, void *);
                print64((uint64_t)p, 16, false, digit, PRINT_0X);
                break;
            case 'l':
            case 'L':
                fmt++;
                switch(c = *fmt) {
                case 'd':
                    print64(va_arg(ap, int64_t), 10, true, digit, zero_padding ? ZERO_PADDING : 0);
                    break;
                case 'u':
                    print64(va_arg(ap, uint64_t), 10, false, digit, zero_padding ? ZERO_PADDING : 0);
                    break;
                case 'x':
                    print64(va_arg(ap, uint64_t), 16, false, digit, zero_padding ? ZERO_PADDING : 0);
                    break;
                default:
                    putchar('%');
                    putchar('l');
                    putchar(c);
                    break;
                }
                break;
            case 'c':
                putchar(va_arg(ap, int));
                break;
            case 's':
                s = va_arg(ap, char *);
                if(!s)
                    s = "(null)";
                puts(s);
                break;
            case '%':
                putchar('%');
                break;
            default:
                putchar('%');
                putchar(c);
                break;
            }
        } else {
            putchar(c);
        }
    }

    return 0;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    return 0;
}

int __printf_chk(int flag, const char *fmt, ...)
{
    va_list ap;

    (void)flag;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    return 0;
}

int fflush(FILE *stream)
{
    (void)stream;
    // Implement flush if necessary for your output device.
    // This is a placeholder implementation.
    pl011_flush();
    return 0;
}

void abort(void)
{
    panic(NULL);
}