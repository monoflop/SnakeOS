#include "serial.h"
#include "stdio.h"
#include "stdarg.h"

//@TODO very crude :(
void init_serial()
{
	outportb(COM1 + 1, 0x00);    // Disable all interrupts
	outportb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outportb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outportb(COM1 + 1, 0x00);    //                  (hi byte)
	outportb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
	outportb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outportb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received()
{
	return inportb(COM1 + 5) & 1;
}

char read_serial()
{
	while (serial_received() == 0);

	return inportb(COM1);
}

int is_transmit_empty()
{
	return inportb(COM1 + 5) & 0x20;
}

void write_serial(char a)
{
	while (is_transmit_empty() == 0);

	outportb(COM1, a);
}
