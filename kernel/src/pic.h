#pragma once

#include <stdint.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

void pic_setup(void);
void pic_set_irq_keyboard_mask(void);
void pic_send_eoi(uint8_t irq);
