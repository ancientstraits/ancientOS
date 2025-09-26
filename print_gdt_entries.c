#include <stdio.h>
#include <stdint.h>

#define MAKE_GDT_DESCRIPTOR_UPPERHALF(base, limit, access_byte, flags) ( \
        ((base >> 16) & 0x00FF) | (access_byte << 8) | (limit & 0xF0000) \
        | (flags << 20) | (base & 0xFF000000) \
    )

#define MAKE_GDT_DESCRIPTOR_LOWERHALF(base, limit, access_byte, flags) \
    ((limit & 0x0FFFF) | ((base << 16) & 0x0000FFFF))

#define MAKE_GDT_DESCRIPTOR(base, limit, access_byte, flags) ( \
        ((uint64_t)(MAKE_GDT_DESCRIPTOR_UPPERHALF(base, limit, access_byte, flags)) << 32) \
        | MAKE_GDT_DESCRIPTOR_LOWERHALF(base, limit, access_byte, flags) \
    )

uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
    uint64_t descriptor;
 
    // Create the high 32 bit segment
    descriptor  =  limit       & 0x000F0000;         // set limit bits 19:16
    descriptor |= (flag <<  8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
    descriptor |= (base >> 16) & 0x000000FF;         // set base bits 23:16
    descriptor |=  base        & 0xFF000000;         // set base bits 31:24
 
    // Shift by 32 to allow for low part of segment
    descriptor <<= 32;
 
    // Create the low 32 bit segment
    descriptor |= base  << 16;                       // set base bits 15:0
    descriptor |= limit  & 0x0000FFFF;               // set limit bits 15:0
 
    return descriptor;
}

int main() {
    uint64_t a = MAKE_GDT_DESCRIPTOR(0, 0xFFFFF, 0x9A, 0xA);
    uint64_t b = MAKE_GDT_DESCRIPTOR(0, 0xFFFFF, 0x92, 0xC);
    uint64_t c = MAKE_GDT_DESCRIPTOR(0, 0xFFFFF, 0xFA, 0xA);
    uint64_t d = MAKE_GDT_DESCRIPTOR(0, 0xFFFFF, 0xF2, 0xC);

    printf("0x%.16llX\n", a);
    printf("0x%.16llX\n", b);
    printf("0x%.16llX\n", c);
    printf("0x%.16llX\n", d);
}
