#include "kernel.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void pit_init(uint32_t hz)
{
    if (hz < 19 || hz > 1000)
        hz = 100;
    uint32_t divisor = 1193182u / hz;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xff));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xff));
}
