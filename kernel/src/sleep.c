#include "common.h"
#include "sleep.h"

// we're using PIT

#define PIT_HERTZ 1193182
#define PIT_CH0 0x40
#define PIT_CH1 0x41
#define PIT_CH2 0x42
#define PIT_CMD 0x43

void sleep_init(void) {
    outb(PIT_CMD, 0b10110010);
}

void sleep(uint32_t ms) {
    uint32_t ticks = (PIT_HERTZ * ms) / 1000;
}
