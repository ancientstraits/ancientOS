#pragma once

#include <stdint.h>

void setup_idt(void);
void idt_set_descriptor(uint8_t idx, void* isr, uint8_t attrs);
