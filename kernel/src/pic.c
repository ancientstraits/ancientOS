#include <stdint.h>
#include "common.h"
#include "pic.h"

void pic_setup(void) {
    // initialize both PICs
    // TODO do I need io_wait()?
    outb(PIC1_CMD, 0x11);
    io_wait();
    outb(PIC2_CMD, 0x11);
    io_wait();

    // Set the offsets for PIC1 and PIC2
    // PIC1 handles the first 8 interrupts
    // PIC2 handles the last 8 interrupts
    // Interrupts 32 to 39 = for PIC1
    // Interrupts 40 to 47 = for PIC2
    outb(PIC1_CMD, 32);
    io_wait();
    outb(PIC2_CMD, 40);
    io_wait();

    // PIC1 is the master, PIC2 is the slave,
    // and they are connected at IRQ 2
    outb(PIC1_DATA, 1 << 2);
    io_wait();
    outb(PIC1_DATA, 2);
    io_wait();

    // Set both PICs to 8086 mode
    outb(PIC1_DATA, 1);
    io_wait();
    outb(PIC2_DATA, 1);
    io_wait();

    // Unmask them
    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

void pic_send_eoi(uint8_t irq) {
    outb((irq < 8) ? PIC1_CMD : PIC2_CMD, PIC_EOI);
}

void pic_set_irq_keyboard_mask(void) {
    // Only unmask IRQ1 (the keyboard IRQ)
    outb(PIC1_DATA, 0b11111101);
    outb(PIC2_DATA, 0b11111111);
}

void pic_set_irq_mask(uint8_t irq) {
    uint8_t data_port;
    if (irq < 8) {
        data_port = PIC1_DATA;
    } else {
        data_port = PIC2_DATA;
        irq -= 8;
    }

    uint8_t old_mask = inb(data_port);
    uint8_t new_mask = old_mask | (1 << irq);
    outb(data_port, new_mask);
}

void pic_clear_irq_mask(uint8_t irq) {
    uint8_t data_port;
    if (irq < 8) {
        data_port = PIC1_DATA;
    } else {
        data_port = PIC2_DATA;
        irq -= 8;
    }

    uint8_t old_mask = inb(data_port);
    uint8_t new_mask = old_mask & ~(1 << irq);
    outb(data_port, new_mask);
}


