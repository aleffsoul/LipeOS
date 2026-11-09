#include "kernel.h"

#define COM1 0x3f8

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial_raw(char ch)
{
    uint32_t timeout = 1000000;
    while (!(inb(COM1 + 5) & 0x20) && --timeout)
        __asm__ volatile ("pause");
    if (timeout)
        outb(COM1, (uint8_t)ch);
}

void serial_init(void)
{
    outb(COM1 + 1, 0x00); /* Disable UART interrupts. */
    outb(COM1 + 3, 0x80); /* Enable divisor latch. */
    outb(COM1 + 0, 0x03); /* 38400 baud divisor. */
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03); /* 8 data bits, no parity, one stop bit. */
    outb(COM1 + 2, 0xc7); /* FIFO. */
    outb(COM1 + 4, 0x0b); /* DTR, RTS, OUT2. */
}

void serial_putc(char ch)
{
    if (ch == '\n')
        serial_raw('\r');
    serial_raw(ch);
}

void serial_write(const char *text)
{
    while (*text)
        serial_putc(*text++);
}
