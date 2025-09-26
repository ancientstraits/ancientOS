#pragma once

#include <stdint.h>
#define QEMU_DEBUGCON 0xE9

// TODO I wanna implement this whole thing in assembly :(

// inline asm functions
void outb(uint16_t io_port, uint8_t byte);
uint8_t inb(uint16_t io_port);
void pause(void);

// io functions
void putchar(char c);
void puts(const char* str);
void putbyte(uint8_t byte);
void putshort(uint16_t num);
void putint(uint32_t num);
void putlong(uint64_t num);
