#include "kernel.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void io_wait(void)
{
    outb(0x80, 0);
}

void pic_init(void)
{
    uint8_t master_mask = 0xff;
    uint8_t slave_mask = 0xff;

    outb(0x20, 0x11); io_wait();
    outb(0xa0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait(); /* Master IRQs -> vectors 32..39. */
    outb(0xa1, 0x28); io_wait(); /* Slave IRQs -> vectors 40..47. */
    outb(0x21, 0x04); io_wait();
    outb(0xa1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xa1, 0x01); io_wait();

    /* Unmask only PIT IRQ0 and keyboard IRQ1. */
    master_mask &= (uint8_t)~0x03;
    outb(0x21, master_mask);
    outb(0xa1, slave_mask);
}

void pic_eoi_master(void)
{
    outb(0x20, 0x20);
}

void pic_eoi_slave(void)
{
    outb(0xa0, 0x20);
    outb(0x20, 0x20);
}
