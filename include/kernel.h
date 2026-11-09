#ifndef LIPEOS_KERNEL_H
#define LIPEOS_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#define MULTIBOOT2_BOOT_MAGIC 0x36d76289u

void serial_init(void);
void serial_write(const char *text);
void serial_putc(char ch);
void console_init(void);
void console_clear(void);
void console_putc(char ch);
void console_write(const char *text);
void console_write_line(const char *text);
void idt_init(void);
void pic_init(void);
void pic_eoi_master(void);
void pic_eoi_slave(void);
void pit_init(uint32_t hz);
void keyboard_init(void);
void keyboard_irq_handler(void);
int keyboard_try_pop(char *out);
void timer_irq_handler(void);
void unhandled_irq_handler(void);
void exception_handler(void);
void shell_handle_char(char ch);

static inline void cpu_halt(void)
{
    __asm__ volatile ("hlt");
}

static inline void cpu_cli(void)
{
    __asm__ volatile ("cli" ::: "memory");
}

static inline void cpu_sti(void)
{
    __asm__ volatile ("sti" ::: "memory");
}

#endif
