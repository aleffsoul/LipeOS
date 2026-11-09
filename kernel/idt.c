#include "kernel.h"

struct idt_gate {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_register {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_gate idt[256] __attribute__((aligned(16)));

extern void isr_exception(void);
extern void isr_default(void);
extern void isr_timer(void);
extern void isr_keyboard(void);

static void idt_set_gate(uint8_t vector, void (*handler)(void))
{
    uint64_t address = (uint64_t)handler;
    idt[vector].offset_low = (uint16_t)(address & 0xffff);
    idt[vector].selector = 0x08;
    idt[vector].ist = 0;
    idt[vector].attributes = 0x8e; /* Present, ring 0, 64-bit interrupt gate. */
    idt[vector].offset_middle = (uint16_t)((address >> 16) & 0xffff);
    idt[vector].offset_high = (uint32_t)(address >> 32);
    idt[vector].reserved = 0;
}

void idt_init(void)
{
    for (uint16_t i = 0; i < 256; ++i)
        idt_set_gate((uint8_t)i, isr_default);
    for (uint8_t i = 0; i < 32; ++i)
        idt_set_gate(i, isr_exception);
    idt_set_gate(32, isr_timer);
    idt_set_gate(33, isr_keyboard);

    struct idt_register idtr = {
        .limit = (uint16_t)(sizeof(idt) - 1),
        .base = (uint64_t)idt,
    };
    __asm__ volatile ("lidt %0" : : "m"(idtr) : "memory");
    serial_write("[idt] 256 interrupt gates installed; PIT=32, keyboard=33\n");
}
