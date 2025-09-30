#include "common.h"

void outb(uint16_t io_port, uint8_t byte) {
    __asm__ volatile("out %0, %1"
        :
        : "Nd"(io_port), "a"(byte)
        : "memory"
    );
}
uint8_t inb(uint16_t io_port) {
    uint8_t ret;
    __asm__ volatile("in %0, %1"
        : "=a"(ret)
        : "Nd"(io_port)
        : "memory"
    );

    return ret;
}
void pause(void) {
    __asm__ volatile("pause");
}
void io_wait(void) {
    // this operation does nothing, but it gives a little delay
    outb(0x80, 0);
}
void enable_interrupts(void) {
    __asm__ volatile("sti");
}
void disable_interrupts(void) {
    __asm__ volatile("cli");
}

void putchar(char c) {
    outb(QEMU_DEBUGCON, c);
}
void puts(const char* str) {
    for (; *str; str++)
        putchar(*str);
    putchar('\n');
}

// these macros are so cool. I just hope they aren't slow...
#define NIBBLE_AS_CHAR(nib) (((nib) <= 9) ? ('0' + (nib)) : ('A' - 10 + (nib)))
#define GET_NIBBLE(num, n) (((num) >> (4 * (n))) & 0xF)
#define PUT4(num, n)  putchar(NIBBLE_AS_CHAR(GET_NIBBLE((num), (n))))
#define PUT8(num, n)  (PUT4( (num), 1 + 2 * (n)), PUT4( (num), 0 + 2 * (n)))
#define PUT16(num, n) (PUT8( (num), 1 + 2 * (n)), PUT8( (num), 0 + 2 * (n)))
#define PUT32(num, n) (PUT16((num), 1 + 2 * (n)), PUT16((num), 0 + 2 * (n)))
#define PUT64(num, n) (PUT32((num), 1 + 2 * (n)), PUT32((num), 0 + 2 * (n)))

void putbyte(uint8_t byte) {
    putchar('0');
    putchar('x');
    PUT8(byte, 0);
    putchar('\n');
}
void putshort(uint16_t num) {
    putchar('0');
    putchar('x');
    PUT16(num, 0);
    putchar('\n');    
}
void putint(uint32_t num) {
    putchar('0');
    putchar('x');
    PUT32(num, 0);
    putchar('\n');    
}
void putlong(uint64_t num) {
    putchar('0');
    putchar('x');
    PUT64(num, 0);
    putchar('\n');    
}

// Halt and catch fire function.
void hcf(void) {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}
