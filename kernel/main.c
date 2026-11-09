#include "kernel.h"

#define INPUT_CAPACITY 128

static char command_line[INPUT_CAPACITY];
static size_t command_length;
static volatile uint64_t timer_ticks;

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial_hex64(uint64_t value)
{
    static const char digits[] = "0123456789abcdef";
    serial_write("0x");
    for (int shift = 60; shift >= 0; shift -= 4)
        serial_putc(digits[(value >> shift) & 0xf]);
}

static int string_equal(const char *left, const char *right)
{
    while (*left && *right) {
        char a = *left++;
        char b = *right++;
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) return 0;
    }
    return *left == 0 && *right == 0;
}

static void print_prompt(void)
{
    console_write("LipeOS> ");
}

static void shell_execute(void)
{
    command_line[command_length] = 0;
    if (string_equal(command_line, "help")) {
        console_write_line("Comandos: help, clear, about, reboot");
    } else if (string_equal(command_line, "clear")) {
        console_clear();
        serial_write("\033[2J\033[H");
    } else if (string_equal(command_line, "about")) {
        console_write_line("LipeOS x86_64 - kernel freestanding Multiboot2.");
        console_write_line("Versao inicial para testes em maquinas virtuais.");
    } else if (string_equal(command_line, "reboot")) {
        console_write_line("Reiniciando via controlador de teclado...");
        serial_write("[shell] reboot requested\n");
        cpu_cli();
        uint32_t timeout = 1000000;
        while ((inb(0x64) & 0x02) && --timeout)
            __asm__ volatile ("pause");
        __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0xfe), "Nd"((uint16_t)0x64));
        for (;;) cpu_halt();
    } else if (command_length != 0) {
        console_write_line("Comando desconhecido. Digite help.");
    }
    command_length = 0;
    print_prompt();
}

void shell_handle_char(char ch)
{
    if (ch == '\n') {
        console_putc('\n');
        shell_execute();
        return;
    }
    if (ch == '\b') {
        if (command_length) {
            --command_length;
            console_putc('\b');
        }
        return;
    }
    if (command_length + 1 < sizeof(command_line)) {
        command_line[command_length++] = ch;
        console_putc(ch);
    }
}

void timer_irq_handler(void)
{
    ++timer_ticks;
    pic_eoi_master();
}

void unhandled_irq_handler(void)
{
    pic_eoi_master();
}

void exception_handler(void)
{
    serial_write("[fault] CPU exception; kernel halted\n");
    console_write_line("LipeOS: excecao de CPU. Sistema parado.");
    cpu_cli();
    for (;;) cpu_halt();
}

static void report_multiboot_info(uint64_t info_address)
{
    if (!info_address || info_address >= 0x40000000ull) {
        serial_write("[multiboot2] invalid information pointer\n");
        return;
    }
    const uint32_t *info = (const uint32_t *)(uintptr_t)info_address;
    uint32_t total_size = info[0];
    if (total_size < 16 || total_size > 0x100000) {
        serial_write("[multiboot2] invalid information block size\n");
        return;
    }
    serial_write("[multiboot2] information block at ");
    serial_hex64(info_address);
    serial_write(", bytes=");
    serial_hex64(total_size);
    serial_putc('\n');
}

void kernel_main(uint64_t magic, uint64_t multiboot_info)
{
    serial_init();
    console_init();
    serial_write("[boot] LipeOS x86_64 kernel entered\n");

    if ((uint32_t)magic != MULTIBOOT2_BOOT_MAGIC) {
        serial_write("[boot] invalid Multiboot2 magic: ");
        serial_hex64(magic);
        serial_putc('\n');
        console_write_line("Erro: inicializacao Multiboot2 invalida.");
        for (;;) cpu_halt();
    }
    serial_write("[multiboot2] valid GRUB handoff\n");
    report_multiboot_info(multiboot_info);

    idt_init();
    pic_init();
    pit_init(100);
    keyboard_init();
    serial_write("[interrupts] PIC remapped; PIT and PS/2 keyboard ready\n");

    console_write_line("LipeOS iniciado com sucesso");
    console_write_line("Kernel x86_64 / GRUB2 Multiboot2");
    print_prompt();
    cpu_sti();

    for (;;) {
        char ch;
        cpu_cli();
        if (keyboard_try_pop(&ch)) {
            cpu_sti();
            shell_handle_char(ch);
        } else {
            __asm__ volatile ("sti; hlt" ::: "memory");
        }
    }
}
