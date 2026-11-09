#include "kernel.h"

#define KEY_QUEUE_SIZE 128

static volatile char key_queue[KEY_QUEUE_SIZE];
static volatile uint8_t key_head;
static volatile uint8_t key_tail;

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static char scancode_to_ascii(uint8_t code)
{
    switch (code) {
    case 0x02: return '1'; case 0x03: return '2'; case 0x04: return '3';
    case 0x05: return '4'; case 0x06: return '5'; case 0x07: return '6';
    case 0x08: return '7'; case 0x09: return '8'; case 0x0a: return '9';
    case 0x0b: return '0';
    case 0x10: return 'q'; case 0x11: return 'w'; case 0x12: return 'e';
    case 0x13: return 'r'; case 0x14: return 't'; case 0x15: return 'y';
    case 0x16: return 'u'; case 0x17: return 'i'; case 0x18: return 'o';
    case 0x19: return 'p';
    case 0x1e: return 'a'; case 0x1f: return 's'; case 0x20: return 'd';
    case 0x21: return 'f'; case 0x22: return 'g'; case 0x23: return 'h';
    case 0x24: return 'j'; case 0x25: return 'k'; case 0x26: return 'l';
    case 0x2c: return 'z'; case 0x2d: return 'x'; case 0x2e: return 'c';
    case 0x2f: return 'v'; case 0x30: return 'b'; case 0x31: return 'n';
    case 0x32: return 'm';
    case 0x39: return ' ';
    case 0x0e: return '\b';
    case 0x1c: return '\n';
    default: return 0;
    }
}

void keyboard_init(void)
{
    uint32_t timeout = 100000;
    while ((inb(0x64) & 0x02) && --timeout)
        __asm__ volatile ("pause");
    if (timeout)
        __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0xae), "Nd"((uint16_t)0x64));
}

void keyboard_irq_handler(void)
{
    uint8_t code = inb(0x60);
    if (!(code & 0x80)) {
        char ch = scancode_to_ascii(code);
        if (ch) {
            uint8_t next = (uint8_t)((key_head + 1) & (KEY_QUEUE_SIZE - 1));
            if (next != key_tail) {
                key_queue[key_head] = ch;
                key_head = next;
            }
        }
    }
    pic_eoi_master();
}

int keyboard_try_pop(char *out)
{
    if (key_tail == key_head)
        return 0;
    *out = key_queue[key_tail];
    key_tail = (uint8_t)((key_tail + 1) & (KEY_QUEUE_SIZE - 1));
    return 1;
}
