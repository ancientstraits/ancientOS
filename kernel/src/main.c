#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "common.h"
// #include "asm.h"
#include "gdt.h"

#define QEMU_DEBUGCON 0xE9
#define I8042_DATA 0x60
#define I8042_STATCMD 0x64

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
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

void send_to_port1(uint8_t byte) {
    while ((inb(I8042_STATCMD) & 0b10) != 0);
    outb(I8042_DATA, byte);
}
void send_to_port2(uint8_t byte) {
    outb(I8042_STATCMD, 0xD4);
    while ((inb(I8042_STATCMD) & 0b10) != 0);
    outb(I8042_DATA, byte);
}

uint8_t poll_byte() {
    while ((inb(I8042_STATCMD) & 0b1) == 0);
    return inb(I8042_DATA);
}

void keyboard() {
    // disable devices
    outb(I8042_STATCMD, 0xAD); // disable 1st PS/2 port
    outb(I8042_STATCMD, 0xA7); // disable 2nd PS/2 port

    // flush the output buffer
    // The 0th bit is 1 if the output buffer is full
    while (inb(I8042_STATCMD) & 0b1) {
        inb(I8042_DATA); // get byte from data and discard
    }

    // set the controller configuration byte
    outb(I8042_STATCMD, 0x20);
    uint8_t cconf = inb(I8042_DATA);
    cconf &= ~0b01010001; // Clear bits 0, 4, and 6
    outb(I8042_STATCMD, 0x60);
    outb(I8042_DATA, cconf);

    // perform controller self-text
    outb(I8042_STATCMD, 0xAA);
    // while ((inb(I8042_STATCMD) & 0b1) == 0);
    uint8_t test_output = inb(I8042_DATA);
    puts("Did self-test work?");
    puts((test_output == 0x55) ? "Yes" : "No");
    if (test_output != 0x55) hcf();
    outb(I8042_STATCMD, 0x60); // set cconf byte again,
    outb(I8042_DATA, cconf);   // because it might have been erased

    // determine if there are 2 channels
    outb(I8042_STATCMD, 0xA8); // Enable 2nd PS/2 port
    outb(I8042_STATCMD, 0x20); // read cconf byte again 
    bool is_dual_channel = ((inb(I8042_DATA) >> 5) & 1) == 0; // Is dual if bit 5 is 0
    outb(I8042_STATCMD, 0xA8); // Disable 2nd PS/2 port
    if (is_dual_channel) {
        cconf &= ~0b00100010; // Clear bits 1 and 5
        outb(I8042_STATCMD, cconf); // set cconf byte again
        outb(I8042_DATA, cconf);
    }
    puts("Is dual channel?");
    puts(is_dual_channel ? "Yes" : "No");

    // flush the output buffer AGAIN
    // The 0th bit is 1 if the output buffer is full
    puts("FLUSH START!");
    while (inb(I8042_STATCMD) & 0b1) {
        putbyte(inb(I8042_DATA)); // get byte from data and print
    }
    puts("FLUSH END.");

    // interface tests
    bool port1_works = false, port2_works = false;

    outb(I8042_STATCMD, 0xAB);
    while ((inb(I8042_STATCMD) & 0b1) == 0);
    uint8_t test1 = inb(I8042_DATA);
    port1_works = (test1 == 0);
    puts("1st channel interface test worked?");
    puts((test1 == 0) ? "Yes" : "No");
    if (test1 != 0) putbyte(test1);
    if (is_dual_channel) {
        outb(I8042_STATCMD, 0xA9);
        while ((inb(I8042_STATCMD) & 0b1) == 0);
        uint8_t test2 = inb(I8042_DATA);
        port2_works = (test2 == 0);
        puts("2nd channel interface test worked?");
        puts((test2 == 0) ? "Yes" : "No");
        if (test2 != 0) putbyte(test2);
        // if (test2 != 0) hcf();
    }

    if (!port1_works && !port2_works) {
        puts("Neither port works. Catching fire...");
        hcf();
    }

    // enable devices
    if (port1_works) {
        outb(I8042_STATCMD, 0xAE);
    }
    if (port2_works) {
        outb(I8042_STATCMD, 0xA8);
    }

    // reset devices
    if (port1_works) {
        send_to_port1(0xFF);
        uint8_t byte_1 = poll_byte();
        if (byte_1 == 0xFC) {
            puts("Port 1 no work");
            hcf();
        }
        uint8_t byte_2 = poll_byte();
        uint8_t byte_3 = poll_byte();
        puts("Port 1:");
        putbyte(byte_1);
        putbyte(byte_2);
        putbyte(byte_3);
    }
    if (port2_works) {
        send_to_port2(0xFF);
        uint8_t byte_1 = poll_byte();
        if (byte_1 == 0xFC) {
            puts("Port 2 no work");
            hcf();
        }
        uint8_t byte_2 = poll_byte();
        uint8_t byte_3 = poll_byte();
        puts("Port 2:");
        putbyte(byte_1);
        putbyte(byte_2);
        putbyte(byte_3);
    }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    puts("Loading GDT...");
    setup_gdt();
    puts("GDT Loaded!");

    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++) {
        volatile uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    puts("I am OS.");
    // putbyte_asm(0x00);
    // putbyte_asm(0xFF);
    // putbyte_asm(0x12);
    // putbyte_asm(0x34);
    // putbyte_asm(0xDE);
    // putbyte_asm(0xAD);
    // putbyte_asm(0xBE);
    // putbyte_asm(0xEF);
    // putbyte_asm(0x67);
    // keyboard();

    // We're done, just hang...
    hcf();
}
