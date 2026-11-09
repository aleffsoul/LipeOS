#include "kernel.h"

#define VGA_COLUMNS 80
#define VGA_ROWS 25
#define VGA_MEMORY ((volatile uint16_t *)0xb8000)
#define VGA_ATTRIBUTE 0x0f

static uint32_t cursor_x;
static uint32_t cursor_y;

static void scroll_if_needed(void)
{
    if (cursor_y < VGA_ROWS)
        return;

    for (uint32_t y = 1; y < VGA_ROWS; ++y) {
        for (uint32_t x = 0; x < VGA_COLUMNS; ++x)
            VGA_MEMORY[(y - 1) * VGA_COLUMNS + x] = VGA_MEMORY[y * VGA_COLUMNS + x];
    }
    for (uint32_t x = 0; x < VGA_COLUMNS; ++x)
        VGA_MEMORY[(VGA_ROWS - 1) * VGA_COLUMNS + x] = (VGA_ATTRIBUTE << 8) | ' ';
    cursor_y = VGA_ROWS - 1;
}

void console_clear(void)
{
    for (uint32_t i = 0; i < VGA_COLUMNS * VGA_ROWS; ++i)
        VGA_MEMORY[i] = (VGA_ATTRIBUTE << 8) | ' ';
    cursor_x = 0;
    cursor_y = 0;
}

void console_init(void)
{
    console_clear();
}

void console_putc(char ch)
{
    serial_putc(ch);

    if (ch == '\r') {
        cursor_x = 0;
        return;
    }
    if (ch == '\n') {
        cursor_x = 0;
        ++cursor_y;
        scroll_if_needed();
        return;
    }
    if (ch == '\b') {
        if (cursor_x) {
            --cursor_x;
            VGA_MEMORY[cursor_y * VGA_COLUMNS + cursor_x] = (VGA_ATTRIBUTE << 8) | ' ';
        }
        return;
    }
    if ((unsigned char)ch < 0x20)
        return;

    VGA_MEMORY[cursor_y * VGA_COLUMNS + cursor_x] = (VGA_ATTRIBUTE << 8) | (uint8_t)ch;
    if (++cursor_x == VGA_COLUMNS) {
        cursor_x = 0;
        ++cursor_y;
        scroll_if_needed();
    }
}

void console_write(const char *text)
{
    while (*text)
        console_putc(*text++);
}

void console_write_line(const char *text)
{
    console_write(text);
    console_putc('\n');
}
