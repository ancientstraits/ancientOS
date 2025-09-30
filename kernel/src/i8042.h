#pragma once

#define I8042_DATA 0x60
#define I8042_STATCMD 0x64

void i8042_setup();
void i8042_interrupt_handler(void);
