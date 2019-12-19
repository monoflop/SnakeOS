#ifndef STDIO_H
#define STDIO_H

#define KERNEL_COM_OUTPUT

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#include "stdlib.h"
#include "stdint.h"

void cls(void);
void putc(char c);
void puts(const char* s);

int printf(const char* fmt, ...);

void *memset(void *b, int c, int len);
void memcpy(void * destination, const void * source, size_t num);

uint8_t inportb(uint16_t _port);
void outportb(uint16_t _port, uint8_t _data);
uint16_t inport(uint16_t _port);
void outport(uint16_t _port, uint16_t _data);

#endif
