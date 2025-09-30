#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "idt.h"
#include "pic.h"
#include "i8042.h"


void send_to_port1(uint8_t byte) {
    while ((inb(I8042_STATCMD) & 0b10) != 0);
    puts("AAAA");
    outb(I8042_DATA, byte);
    puts("BBBB");
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

static void flush() {
    // The 0th bit is 1 if the output buffer is full
    puts("FLUSH START!");
    while (inb(I8042_STATCMD) & 0b1) {
        putbyte(inb(I8042_DATA)); // get byte from data and print
    }
    puts("FLUSH END.");
}

void i8042_setup() {
    // disable devices
    outb(I8042_STATCMD, 0xAD); // disable 1st PS/2 port
    outb(I8042_STATCMD, 0xA7); // disable 2nd PS/2 port

    flush();

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
        outb(I8042_STATCMD, 0x60); // set cconf byte again
        outb(I8042_DATA, cconf);
    }
    puts("Is dual channel?");
    puts(is_dual_channel ? "Yes" : "No");

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
    puts("Enabling devices...");
    if (port1_works) {
        outb(I8042_STATCMD, 0xAE);
    }
    if (port2_works) {
        outb(I8042_STATCMD, 0xA8);
    }

    puts("Enabling keyboard interrupt...");
    cconf |= 0b00000001;
    outb(I8042_STATCMD, 0x60); // set cconf byte
    outb(I8042_DATA, cconf);

    // reset devices
    puts("Resetting devices...");
    if (port1_works) {
        puts("A");
        send_to_port1(0xFF);
        puts("B");
        uint8_t byte_1 = poll_byte();
        if (byte_1 == 0xFC) {
            puts("Port 1 no work");
            hcf();
        }
        puts("C");
        uint8_t byte_2 = poll_byte();
        puts("D");
        // uint8_t byte_3 = poll_byte();
        puts("Port 1:");
        putbyte(byte_1);
        putbyte(byte_2);
        // putbyte(byte_3);
    }
    if (port2_works) {
        send_to_port2(0xFF);
        uint8_t byte_1 = poll_byte();
        if (byte_1 == 0xFC) {
            puts("Port 2 no work");
            hcf();
        }
        uint8_t byte_2 = poll_byte();
        // uint8_t byte_3 = poll_byte();
        puts("Port 2:");
        putbyte(byte_1);
        putbyte(byte_2);
        // putbyte(byte_3);
    }

    puts("One last flush...");
    flush();
}

void i8042_interrupt_handler(void) {
    uint8_t status = inb(I8042_STATCMD);
    if (status & 1) {
        uint8_t scancode = inb(I8042_DATA);
        putbyte(scancode);
    }


    outb(PIC1_CMD, PIC_EOI);
}
