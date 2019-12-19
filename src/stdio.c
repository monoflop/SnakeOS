/* PUTN
 * The Minimal snprintf() implementation
 *
 * Copyright (c) 2013,2014 Michal Ludvig <michal@logix.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the auhor nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----
 *
 * This is a minimal snprintf() implementation optimised
 * for embedded systems with a very limited program memory.
 * mini_snprintf() doesn't support _all_ the formatting
 * the glibc does but on the other hand is a lot smaller.
 * Here are some numbers from my STM32 project (.bin file size):
 *      no snprintf():      10768 bytes
 *      mini snprintf():    11420 bytes     (+  652 bytes)
 *      glibc snprintf():   34860 bytes     (+24092 bytes)
 * Wasting nearly 24kB of memory just for snprintf() on
 * a chip with 32kB flash is crazy. Use mini_snprintf() instead.
 *
 */

#include "stdio.h"

#include "stdarg.h"
#include "stdint.h"
#include "stdlib.h"
#include "serial.h"

static char* BASEADDRESS = (char*)0xb8000;
static int printf_res = 0;

static int x = 0;
static int y = 0;

void cls(void)
{
	x = 0;
	y = 0;
	memset(BASEADDRESS, 0x0, 4192);
}

void putc(char c)
{
    if((c == '\n') || (x > 79))
	{
        x = 0;
        y++;

		#ifdef KERNEL_COM_OUTPUT
			write_serial('\n');
		#endif
    }

    if (c == '\n')
	{
        return;
    }

    if (y > 24)
	{
        int i;
        for (i = 0; i < 2 * 24 * 80; i++)
		{
            BASEADDRESS[i] = BASEADDRESS[i + 160];
        }

        for (; i < 2 * 25 * 80; i++)
		{
            BASEADDRESS[i] = 0;
        }
        y--;
    }

    BASEADDRESS[2 * (y * 80 + x)] = c;
    BASEADDRESS[2 * (y * 80 + x) + 1] = 0x07;


	#ifdef KERNEL_COM_OUTPUT
		write_serial(c);
	#endif

    x++;
    printf_res++;
}

void puts(const char* s)
{
    while (*s)
	{
        putc(*s++);
    }
}

/*void putn(unsigned long x, int base)
{
    char buf[65];
    const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* p;

    if (base > 36)
	{
        return;
    }

    p = buf + 64;
    *p = '\0';
    do
	{
        *--p = digits[x % base];
        x /= base;
    }
	while (x);
    puts(p);
}*/

static unsigned int putn(long value, unsigned int radix, unsigned int uppercase, unsigned int unsig,
	   unsigned int zero_pad)
{
	char buf[65];
	char *buffer = buf;
	char *pbuffer = buffer;
	int	negative = 0;
	unsigned int i, len;

	if (radix > 16)
		return 0;

	if (value < 0 && !unsig)
	{
		negative = 1;
		value = -value;
	}

	do
	{
		int digit = value % radix;
		*(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
		value /= radix;
	}
	while (value > 0);

	for (i = (pbuffer - buffer); i < zero_pad; i++)
		*(pbuffer++) = '0';

	if (negative)
		*(pbuffer++) = '-';

	*(pbuffer) = '\0';

	len = (pbuffer - buffer);
	for (i = 0; i < len / 2; i++)
	{
		char j = buffer[i];
		buffer[i] = buffer[len - i - 1];
		buffer[len - i - 1] = j;
	}

	buf[len] = '\0';
	puts(buf);

	return len;
}

int printf(const char* fmt, ...)
{
    va_list ap;
    const char* s;
    unsigned long n;

    va_start(ap, fmt);
    printf_res = 0;
    while (*fmt)
	{
        if (*fmt == '%')
		{
			char zero_pad = 0;
            fmt++;


			char ch = (*fmt);
			if (ch == '0')
			{
				fmt++;
				char ch1 = *fmt;
				if(ch1 == '\0') goto out;
				if (ch1 >= '0' && ch1 <= '9') zero_pad = ch1 - '0';
				fmt++;
			}

            switch (*fmt)
			{
                case 's':
                    s = va_arg(ap, char*);
                    puts(s);
                    break;
                case 'd':
                case 'u':
                    n = va_arg(ap, unsigned long int);
                    //putn(n, 10);
					putn(n, 10, (*fmt == 'u'), 0, zero_pad);
                    break;
                case 'x':
				case 'X':
                case 'p':
                    n = va_arg(ap, unsigned long int);
                    //putn(n, 16);
					putn(n, 16, (*fmt == 'X'), 0, zero_pad);
                    break;
                case '%':
                    putc('%');
                    break;
                case '\0':
                    goto out;
                default:
                    putc('%');
                    putc(*fmt);
                    break;
            }
        }
		else
		{
            putc(*fmt);
        }

        fmt++;
    }

out:
    va_end(ap);

    return printf_res;
}

void *memset(void *b, int c, int len)
{
	unsigned char *p = b;
	while(len > 0)
	{
		*p = c;
		p++;
		len--;
	}
	return(b);
}

void memcpy(void * destination, const void * source, size_t num)
{
   // Typecast source and destination addresses to (uint8_t *)
   uint8_t *csrc = (uint8_t *)source;
   uint8_t *cdest = (uint8_t *)destination;

   // Copy contents of source[] to destination[]
   for(int i = 0; i < num; i++)cdest[i] = csrc[i];
}

uint8_t inportb(uint16_t _port)
{
	uint8_t ret;
	__asm__ volatile ("inb %%dx,%%al":"=a" (ret):"d" (_port));
	return ret;
}

void outportb(uint16_t _port, uint8_t _data)
{
	__asm__ volatile ("outb %%al,%%dx": :"d" (_port), "a" (_data));
}

uint16_t inport(uint16_t _port)
{
	uint16_t ret;
	__asm__ volatile ("inb %%dx,%%ax":"=a" (ret):"d" (_port));
	return ret;
}

void outport(uint16_t _port, uint16_t _data)
{
	__asm__ volatile ("outb %%ax,%%dx": :"d" (_port), "a" (_data));
}
