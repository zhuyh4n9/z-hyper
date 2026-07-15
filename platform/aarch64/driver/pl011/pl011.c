#include <utils/console.h>
#include <layout.h>
#include <utils/types.h>

static uintptr_t g_pl011_base_addr;

#define PL011_DR   0x00
#define PL011_FR   0x18
#define PL011_IBRD 0x24
#define PL011_FBRD 0x28
#define PL011_LCRH 0x2C
#define PL011_CR   0x30
#define PL011_IMSC 0x38
#define PL011_ICR  0x44

#define PL011_FR_RXFE (1 << 4)
#define PL011_FR_TXFF (1 << 5)
#define PL011_FR_RXFF (1 << 6)
#define PL011_FR_TXFE (1 << 7)

#define PL011_LCRH_FEN (1 << 4)
#define PL011_LCRH_WLEN_8 (3 << 5)

#define PL011_INTRX_ENABLED (1 << 4)
#define PL011_INTTX_ENABLED (1 << 5)

#define REG(offset) ((volatile uint32_t *)(g_pl011_base_addr + (offset)))

static void pl011_init(uintptr_t base_addr)
{
    g_pl011_base_addr = base_addr;

    uint32_t baud_rate_divisor_int = 27; // For 115200 baud with 48MHz clock
    uint32_t baud_rate_divisor_frac = 8; // For 115200 baud
    // Set baud rate
    // Disable UART
    *REG(PL011_CR) = 0;
    *REG(PL011_IMSC) = 0; // Disable interrupts
    // Enable RX and TX interrupts
    *REG(PL011_IMSC) = PL011_INTRX_ENABLED | PL011_INTTX_ENABLED;
    // Line control: 8 bits, no parity, one stop bit, enable FIFOs
    *REG(PL011_LCRH) = PL011_LCRH_FEN | PL011_LCRH_WLEN_8;
    *REG(PL011_IBRD) = baud_rate_divisor_int;
    *REG(PL011_FBRD) = baud_rate_divisor_frac;
    // Enable UART, TX and RX
    *REG(PL011_CR) = (1 << 0) | (1 << 8) | (1 << 9);
}

static void pl011_putchar(int c)
{
    // Wait until the transmit FIFO is not full
    while (*REG(PL011_FR) & PL011_FR_TXFF) {
        ;
    }
    *REG(PL011_DR) = (uint32_t)c;
}

static int pl011_getchar(void)
{
    // Wait until the receive FIFO is not empty
    while (*REG(PL011_FR) & PL011_FR_RXFE) {
        ;
    }
    return (int)(*REG(PL011_DR) & 0xFF);
}

static void pl011_flush(void)
{
    // Wait until the transmit FIFO is empty
    while (!(*REG(PL011_FR) & PL011_FR_TXFE)) {
        ;
    }
}

static int __pl011_init(const console_t *console)
{
    (void)console;
    pl011_init((uintptr_t)PL011_BASE_ADDR);
    return 0;
}

static int __pl011_putc(const console_t *console, int c)
{
    (void)console;
    pl011_putchar(c);
    return 0;
}

static int __pl011_getc(const console_t *console)
{
    (void)console;
    return pl011_getchar();
}

static int __pl011_flush(const console_t *console)
{
    (void)console;
    pl011_flush();
    return 0;
}

static const console_ops_t s_pl011_ops = {
    .init = __pl011_init,
    .putc = __pl011_putc,
    .getc = __pl011_getc,
    .flush = __pl011_flush,
};

CONSOLE_REGISTER(pl011, 0, &s_pl011_ops);
