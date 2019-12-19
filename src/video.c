#include "video.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "font.h"
#include "stdarg.h"
#include "serial.h"

/////////////////////////////////////////
//Buffered drawing
/////////////////////////////////////////
void m13hb_draw_buffer(uint8_t *buffer, uint64_t size)
{
	memcpy((char *)MODE_13H_MEMORY, buffer, size);
}

void m13hb_cls(uint8_t *buffer)
{
	memset((char *)buffer, 0, (SCREEN_WIDTH * SCREEN_HEIGHT));
}

void m13hb_set_pixel(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color)
{
	memset((char *)buffer + (y << 8) + (y << 6) + x, color, 1);
}

void m13hb_draw_bmp(uint8_t *buffer, uint8_t *img, uint16_t width, uint16_t height, uint16_t x, uint16_t y)
{
	int offset = 0;
	for(int y_count = height; y_count > 0; y_count--)
	{
		memcpy((char *)buffer + ((y_count + y) * SCREEN_WIDTH) + x, img + (width * offset) , width);
		offset++;
	}
}

void m13hb_draw_transparent_bitmap(uint8_t *buffer, uint8_t *img, uint16_t width, uint16_t height, uint16_t x, uint16_t y)
{
	for(int offset = 0; offset < width * height; offset++)
	{
		//36 transparent color
		if(img[offset] != 36)
		{
			m13hb_set_pixel(buffer, x + offset - ((offset / width) * width), y + height - (offset / width), img[offset]);
		}
	}
}

void m13hb_line(uint8_t *buffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color)
{
	int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

	dx = x2-x1;      /* the horizontal distance of the line */
	dy = y2-y1;      /* the vertical distance of the line */
	dxabs = abs(dx);
	dyabs = abs(dy);
	sdx = sgn(dx);
	sdy = sgn(dy);
	x = dyabs >> 1;
	y = dxabs >> 1;
	px = x1;
	py = y1;

	uint8_t *VGA = (uint8_t *)0xA0000;
	VGA[(py << 8)+(py << 6) + px] = color;

	if (dxabs >= dyabs) /* the line is more horizontal than vertical */
	{
		for(i = 0; i < dxabs; i++)
		{
			y += dyabs;
			if(y >= dxabs)
			{
				y -= dxabs;
				py += sdy;
			}

			px += sdx;
			m13hb_set_pixel(buffer, px, py, color);
		}
	}
	else /* the line is more vertical than horizontal */
	{
		for(i = 0; i < dyabs; i++)
		{
			x += dxabs;
			if(x >= dyabs)
			{
				x -= dyabs;
				px += sdx;
			}

			py += sdy;
			m13hb_set_pixel(buffer, px, py, color);
		}
	}
}

void m13hb_print_8x8_character(uint8_t *buffer, uint8_t *character, uint16_t x, uint16_t y, uint8_t color)
{
	for(uint8_t vertical = 0; vertical < 8; vertical++)
	{
		uint8_t reversed_order = 7;
		for(uint8_t horizontal = 0; horizontal < 8; horizontal++)
		{
			if((character[vertical] >> reversed_order) & 0x1)
			{
				 m13hb_set_pixel(buffer, x + horizontal, y + vertical, color);
			}
			reversed_order--;
		}
	}
}

void m13hb_print_8x8_character_background(uint8_t *buffer, uint8_t *character, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color)
{
	for(uint8_t vertical = 0; vertical < 8; vertical++)
	{
		uint8_t reversed_order = 7;
		for(uint8_t horizontal = 0; horizontal < 8; horizontal++)
		{
			if((character[vertical] >> reversed_order) & 0x1)
			{
				m13hb_set_pixel(buffer, x + horizontal, y + vertical, color);
			}
			else
			{
				m13hb_set_pixel(buffer, x + horizontal, y + vertical, bg_color);
			}

			reversed_order--;
		}
	}
}

void m13hb_putc(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color, char c)
{
    m13hb_print_8x8_character_background(buffer, font_data[(uint8_t)c], x, y, color, bg_color);
}

void m13hb_puts(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color, const char* s)
{
	while (*s)
	{
		m13hb_putc(buffer, x, y, color, bg_color, *s++);
		x += 9;
	}
}

void m13hb_putn(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color, unsigned long value, int base)
{
    char buf[65];
    const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* p;

    if (base > 36) {
        return;
    }

    p = buf + 64;
    *p = '\0';
    do {
        *--p = digits[value % base];
        value /= base;
    } while (value);
    m13hb_puts(buffer, x, y, color, bg_color, p);
}

int m13hb_printf(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color, const char* fmt, ...)
{
	va_list ap;
    const char* s;
    unsigned long n;

    va_start(ap, fmt);
    int printf_res = 0;
    while (*fmt)
    {
		if (*fmt == '%')
		{
			fmt++;
			switch (*fmt)
			{
                case 's':
                    s = va_arg(ap, char*);
                    m13hb_puts(buffer, x, y, color, bg_color, s);
                    break;
                case 'd':
                case 'u':
                    n = va_arg(ap, unsigned long int);
                    m13hb_putn(buffer, x, y, color, bg_color, n, 10);
                    break;
                case 'x':
                case 'p':
                    n = va_arg(ap, unsigned long int);
                    m13hb_putn(buffer, x, y, color, bg_color, n, 16);
                    break;
                case '%':
                    m13hb_putc(buffer, x, y, color, bg_color, '%');
                    x += 9;
                    break;
                case '\0':
                    goto out;
                default:
                    m13hb_putc(buffer, x, y, color, bg_color, '%');
                    x += 9;
                    m13hb_putc(buffer, x, y, color, bg_color, *fmt);
                    x += 9;
                    break;
            }
        }
        else
        {
            m13hb_putc(buffer, x, y, color, bg_color, *fmt);
            x += 9;
        }
		fmt++;
    }

out:
    va_end(ap);

    return printf_res;
}

void m13hb_draw_rect(uint8_t *buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t heigth, uint8_t color)
{
	for(int i = 0; i < heigth; i++)
	{
		memset((char *)buffer + x + y * SCREEN_WIDTH + i * SCREEN_WIDTH, color, width);
	}
}

/////////////////////////////////////////
//Direct drawing
/////////////////////////////////////////
void mode_13h_cls()
{
	memset((char *)MODE_13H_MEMORY, 0, (SCREEN_WIDTH * SCREEN_HEIGHT));
}
