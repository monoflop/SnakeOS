#ifndef VIDEO_H
#define VIDEO_H

#include "stdarg.h"
#include "stdint.h"
#include "stdarg.h"

#define MODE_13H_MEMORY 0xA0000
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

void m13hb_draw_buffer(uint8_t *buffer, uint64_t size);
void m13hb_cls(uint8_t *buffer);
void m13hb_set_pixel(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color);
void m13hb_draw_bmp(uint8_t *buffer, uint8_t *img, uint16_t width, uint16_t height, uint16_t x, uint16_t y);
void m13hb_draw_transparent_bitmap(uint8_t *buffer, uint8_t *img, uint16_t width, uint16_t height, uint16_t x, uint16_t y);
void m13hb_line(uint8_t *buffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color);
void m13hb_print_8x8_character(uint8_t *buffer, uint8_t *character, uint16_t x, uint16_t y, uint8_t color);
void m13hb_print_8x8_character_background(uint8_t *buffer, uint8_t *character, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color);
void m13hb_putc(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color, char c);
void m13hb_puts(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color, const char* s);
void m13hb_putn(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color, unsigned long value, int base);
int m13hb_printf(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t color, uint8_t bg_color, const char* fmt, ...);
void m13hb_draw_rect(uint8_t *buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t heigth, uint8_t color);

void mode_13h_cls();
#endif
