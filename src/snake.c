#include "snake.h"

#include "mm.h"
#include "stdlib.h"
#include "stdint.h"
#include "bios_int.h"
#include "video.h"
#include "rtc.h"
#include "stdio.h"

// Because our screen is always 320x200
// we have a playable field of 40x25
#define WORLD_WIDTH 40
#define WORLD_HEIGHT 25

#define KEYBOARD_UP 0x48
#define KEYBOARD_DOWN 0x50
#define KEYBOARD_LEFT 0x4B
#define KEYBOARD_RIGHT 0x4D

#define MENU_TICK_DELAY 250

#define SNAKE_DEFAULT 1
#define SNAKE_HEAD 2
#define SNAKE_FACING_NORTH 4
#define SNAKE_FACING_WEST 8
#define SNAKE_FACING_SOUTH 16
#define SNAKE_FACING_EAST 32

#define SNAKE_DIRECTION_NORTH 1
#define SNAKE_DIRECTION_WEST 2
#define SNAKE_DIRECTION_SOUTH 3
#define SNAKE_DIRECTION_EAST 4

void run_menu(void);
void run_game(void);
void run_game_over(void);
void create_snake_element(uint16_t x, uint16_t y, uint8_t flags, uint8_t direction);
void draw_snake(void);
void translate_snake(void);
void spawn_food(void);
unsigned long maxrand(unsigned long seed, unsigned long max);
void busy_wait(uint32_t duration);

// Sprites
uint8_t sprite_snake_head_0 [64] = {0x24, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x76, 0x2f, 0xf, 0x2f, 0x2f, 0xf, 0x2f, 0x2f, 0x76, 0x2f, 0x0, 0x2f, 0x2f, 0x0, 0x2f, 0x2f, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x76, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x24, 0x24, 0x29, 0x28, 0x24, 0x24, 0x24};
uint8_t sprite_snake_head_1 [64] = {0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x2f, 0x2f, 0xf, 0x0, 0x2f, 0x2f, 0x2f, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x28, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x29, 0x2f, 0x2f, 0xf, 0x0, 0x2f, 0x2f, 0x76, 0x24, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x24, 0x24, 0x24, 0x24, 0x76, 0x76, 0x76, 0x24, 0x24, 0x24};
uint8_t sprite_snake_head_2 [64] = {0x24, 0x24, 0x24, 0x28, 0x29, 0x24, 0x24, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x76, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x2f, 0x2f, 0x0, 0x2f, 0x2f, 0x0, 0x2f, 0x76, 0x2f, 0x2f, 0xf, 0x2f, 0x2f, 0xf, 0x2f, 0x76, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x24};
uint8_t sprite_snake_head_3 [64] = {0x24, 0x24, 0x24, 0x76, 0x76, 0x76, 0x24, 0x24, 0x24, 0x24, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x24, 0x76, 0x2f, 0x2f, 0x0, 0xf, 0x2f, 0x2f, 0x29, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x28, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x2f, 0x2f, 0x2f, 0x0, 0xf, 0x2f, 0x2f, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x24, 0x24};

uint8_t sprite_snake_body_0 [64] = {0x24, 0x24, 0x76, 0x76, 0x76, 0x76, 0x24, 0x24, 0x24, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x2f, 0x76, 0x2f, 0x2f, 0x76, 0x2f, 0x2f, 0x76, 0x2f, 0x76, 0x76, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24};
uint8_t sprite_snake_body_1 [64] = {0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x76, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x24, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x76, 0x76, 0x76, 0x2f, 0x24, 0x24};
uint8_t sprite_snake_body_2 [64] = {0x24, 0x24, 0x76, 0x76, 0x76, 0x76, 0x24, 0x24, 0x24, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x2f, 0x76, 0x76, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24};
uint8_t sprite_snake_body_3 [64] = {0x24, 0x24, 0x2f, 0x76, 0x76, 0x76, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x24, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x2f, 0x2f, 0x76, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x24, 0x24};

uint8_t sprite_apple [64] = {0x24, 0x24, 0x70, 0x28, 0x28, 0x28, 0x24, 0x24, 0x24, 0x70, 0x28, 0x28, 0x28, 0x28, 0x28, 0x24, 0x24, 0x70, 0x28, 0x28, 0xf, 0x28, 0x28, 0x24, 0x24, 0x70, 0x28, 0x28, 0x28, 0xf, 0x28, 0x24, 0x24, 0x24, 0x70, 0x70, 0x28, 0x28, 0x24, 0x24, 0x24, 0x24, 0x24, 0xc0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0xc0, 0x2, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x2, 0x2, 0x24, 0x24};

uint8_t sprite_snake_logo [1024] = {0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x0, 0x0, 0x0, 0x0, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x0, 0x0, 0x0, 0x0, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x0, 0x0, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x0, 0x0, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x0, 0x0, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x0, 0x0, 0x76, 0x76, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x0, 0x0, 0x2f, 0x2f, 0x76, 0x76, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x0, 0x0, 0x2f, 0x2f, 0x76, 0x76, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x3f, 0x3f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x3f, 0x3f, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x3f, 0x3f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x3f, 0x3f, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x76, 0x76, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x2f, 0x2f, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24};

uint8_t sprite_credits [200] = {0x80, 0x24, 0x24, 0x24, 0x80, 0x24, 0x82, 0x82, 0x82, 0x82, 0x24, 0x86, 0x24, 0x24, 0x86, 0x24, 0x88, 0x88, 0x88, 0x88, 0x24, 0x8a, 0x24, 0x24, 0x24, 0x24, 0x8c, 0x8c, 0x8c, 0x8c, 0x24, 0x8d, 0x8d, 0x8d, 0x8d, 0x24, 0x8f, 0x24, 0x24, 0x24, 0x20, 0x24, 0x24, 0x24, 0x20, 0x24, 0x22, 0x24, 0x24, 0x22, 0x24, 0x26, 0x24, 0x24, 0x26, 0x24, 0x28, 0x24, 0x24, 0x28, 0x24, 0x2a, 0x24, 0x24, 0x24, 0x24, 0x2c, 0x24, 0x24, 0x24, 0x24, 0x2d, 0x24, 0x24, 0x2d, 0x24, 0x2f, 0x24, 0x24, 0x24, 0x20, 0x24, 0x20, 0x24, 0x20, 0x24, 0x22, 0x24, 0x24, 0x22, 0x24, 0x26, 0x24, 0x26, 0x26, 0x24, 0x28, 0x24, 0x24, 0x28, 0x24, 0x2a, 0x2a, 0x24, 0x24, 0x24, 0x2c, 0x24, 0x24, 0x24, 0x24, 0x2d, 0x24, 0x24, 0x2d, 0x24, 0x2f, 0x2f, 0x2f, 0x2f, 0x20, 0x20, 0x24, 0x20, 0x20, 0x24, 0x22, 0x24, 0x24, 0x22, 0x24, 0x26, 0x26, 0x24, 0x26, 0x24, 0x28, 0x24, 0x24, 0x28, 0x24, 0x2a, 0x24, 0x24, 0x24, 0x24, 0x2c, 0x24, 0x24, 0x24, 0x24, 0x2d, 0x24, 0x24, 0x2d, 0x24, 0x2f, 0x24, 0x24, 0x2f, 0x20, 0x24, 0x24, 0x24, 0x20, 0x24, 0x22, 0x22, 0x22, 0x22, 0x24, 0x26, 0x24, 0x24, 0x26, 0x24, 0x28, 0x28, 0x28, 0x28, 0x24, 0x2a, 0x2a, 0x2a, 0x2a, 0x24, 0x2c, 0x24, 0x24, 0x24, 0x24, 0x2d, 0x2d, 0x2d, 0x2d, 0x24, 0x2f, 0x2f, 0x2f, 0x2f};

uint8_t sprite_highscore [1156] = {0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2a, 0x2a, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2a, 0x2a, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x2c, 0x2c, 0xf, 0xf, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2b, 0x2b, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, };

struct snake_element {
   uint16_t x;
   uint16_t y;
   uint8_t flags;
   uint8_t direction;
   struct snake_element *next;
};

uint8_t screen_buffer[320 * 200];
bool game_running = true;
uint64_t random_seed = 0;

struct snake_element *head = NULL;

uint8_t snake_direction = SNAKE_DIRECTION_EAST;
uint16_t food_pos_x = 0;
uint16_t food_pos_y = 0;
uint16_t difficulty = 0;
uint16_t score = 0;
uint16_t highscore = 0;
volatile uint64_t tick_counter = 0;
volatile uint8_t last_scancode = 0x0;

void snake_init(void)
{
  // Change videomode through bios interrupt. 320x200x256
  regs16_t regs;
  regs.ax = 0x0013;
  int32(0x10, &regs);
  mode_13h_cls();

  // Setup periodic PIT interrupt
  // 1000 Hz = each ms
  // Each millisecond we increase the tick counter
  int creg = 1193182 / 1000;
  outportb(0x43, 0x34);
  outportb(0x40, creg & 0xFF);
  outportb(0x40, creg >> 8);

  // Seed PRNG
  read_rtc();
  random_seed = seed_from_current_date();
  printf("random seed: %d\n", random_seed);

  // If everything went fine we never return from this method
  // @TODO because we switch game state by calling run_menu / run_game etc.
  // the stack is going to overflow.
  run_menu();

  while(1){}
}

void run_menu(void)
{
  m13hb_cls(screen_buffer);
  m13hb_draw_buffer(screen_buffer, 320 * 200);

  // Draw static stuff
  m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_logo, 32, 32, 144, 50);
  m13hb_draw_transparent_bitmap(screen_buffer, sprite_credits, 40, 5, 275, 190);
  m13hb_printf(screen_buffer, 96, 100, 0x08, 0x00, "i386 Snake Game");
  m13hb_printf(screen_buffer, 65, 130, 0x0f, 0x00, "Press any key to start!");
  m13hb_printf(screen_buffer, 2, 2, 0x0f, 0x00, "HIGH SCORE %d", highscore);
  m13hb_draw_buffer(screen_buffer, 320 * 200);

  last_scancode = 0;

  while(1)
  {
    busy_wait(MENU_TICK_DELAY);

    // Check if any key was pressed
    // printf("last scancode: %d\n", last_scancode);
    if(last_scancode != 0)
    {
      // Any key was pressed or released
      break;
    }
  }

  // Start game
  run_game();
}

void run_game(void)
{
  snake_direction = SNAKE_DIRECTION_EAST;

  // Add starting snake
  create_snake_element(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, SNAKE_HEAD, SNAKE_FACING_EAST);
  create_snake_element(WORLD_WIDTH / 2 - 1, WORLD_HEIGHT / 2, SNAKE_DEFAULT, SNAKE_FACING_EAST);
  create_snake_element(WORLD_WIDTH / 2 - 2, WORLD_HEIGHT / 2, SNAKE_DEFAULT, SNAKE_FACING_EAST);

  // Spawn initial food
  spawn_food();

  // GameLoop
  game_running = true;
  while(game_running == true)
  {
    uint16_t posx = head->x;
    uint16_t posy = head->y;

    // Collision with walls
    if((snake_direction == SNAKE_DIRECTION_NORTH && posy == 2)
      || (snake_direction == SNAKE_DIRECTION_SOUTH && posy == WORLD_HEIGHT - 2)
      || (snake_direction == SNAKE_DIRECTION_WEST && posx == 0)
      || (snake_direction == SNAKE_DIRECTION_EAST && posx == WORLD_WIDTH - 1))
    {
      // Game over
      // @TODO
      printf("GameOver :( Collision with wall");
      run_game_over();
      //while(1){}
      //break;
    }

    // Self collision
    // Set target position
    if(snake_direction == SNAKE_DIRECTION_NORTH)posy--;
    else if(snake_direction == SNAKE_DIRECTION_WEST)posx--;
    else if(snake_direction == SNAKE_DIRECTION_SOUTH)posy++;
    else if(snake_direction == SNAKE_DIRECTION_EAST)posx++;

    struct snake_element *ptr = head;
    while(ptr != NULL)
    {
      if(ptr->x == posx && ptr->y == posy)
      {
        // Game over
        // @TODO
        printf("GameOver :( Collision with snake");
        run_game_over();
        //while(1){}
        //break;
      }
      else ptr = ptr->next;
    }

    // Collision with food
    if(posx == food_pos_x && posy == food_pos_y)
    {
      printf("Collected food");
      difficulty++;
      score += 500;

      // Attach new element
      //@TODO
      ptr = head;
      while(ptr != NULL)
      {
        if(ptr->next == NULL)
        {
          for(int i = 0; i < 2; i++)
          {
            uint16_t new_x = ptr->x;
            uint16_t new_y = ptr->y;
            if(ptr->direction == SNAKE_FACING_NORTH)new_y++;
            else if(ptr->direction == SNAKE_FACING_EAST)new_x--;
            else if(ptr->direction == SNAKE_FACING_SOUTH)new_y--;
            else if(ptr->direction == SNAKE_FACING_WEST)new_x++;
            create_snake_element(new_x, new_y, SNAKE_DEFAULT, ptr->direction);
          }
          break;
        }
        ptr = ptr->next;
      }

      spawn_food();
    }

    //Translate and draw snake and other objects
    translate_snake();

    //Drawing part
    m13hb_cls(screen_buffer);

    draw_snake();
    m13hb_draw_transparent_bitmap(screen_buffer, sprite_apple, 8, 8, food_pos_x * 8, food_pos_y * 8);

    m13hb_printf(screen_buffer, 2, 2, 0x0f, 0x00, "Score: %d", score);
    m13hb_printf(screen_buffer, 200, 2, 0x0f, 0x00, "Cherries: %d", difficulty);
    m13hb_line(screen_buffer, 0, 15, SCREEN_WIDTH, 15, 0x0f);
    m13hb_line(screen_buffer, 0, 195, SCREEN_WIDTH, 195, 0x0f);

    m13hb_draw_buffer(screen_buffer, 320 * 200);

    //Wait
    int delay = 250 - difficulty * 10;
    if(delay < 50)delay = 50;
    busy_wait(delay);
  }
}

void run_game_over(void)
{
  // Draw game over screen and clear all game state
  // Free snake
  struct snake_element *ptr = head;
  while(ptr != NULL)
  {
    ptr = ptr->next;
    mm_free(&ptr);
  }
  head = NULL;

  difficulty = 0;
  food_pos_x = 0;
  food_pos_y = 0;

  // Draw game over screen
  m13hb_draw_rect(screen_buffer, 40, 25, 240, 160, 0x0f);
  m13hb_draw_rect(screen_buffer, 41, 26, 238, 158, 0x00);


  // Only draw if we have a new highscore
  if(score > highscore)
  {
    highscore = score;
    m13hb_printf(screen_buffer, 120, 55, 0x29, 0x00, "Game Over");
    m13hb_draw_transparent_bitmap(screen_buffer, sprite_highscore, 34, 34, 144, 75);
    m13hb_printf(screen_buffer, 60, 120, 0x0f, 0x00, "!!! New High Score !!!");
    m13hb_printf(screen_buffer, 100, 160, 0x08, 0x00, "Press any key!");
  }
  else
  {
    m13hb_printf(screen_buffer, 120, 80, 0x29, 0x00, "Game Over");
    m13hb_printf(screen_buffer, 100, 110, 0x08, 0x00, "Press any key!");
  }

  m13hb_draw_buffer(screen_buffer, 320 * 200);

  score = 0;

  busy_wait(250);

  last_scancode = 0;

  while(1)
  {
    busy_wait(MENU_TICK_DELAY);

    // Check if any key was pressed
    //printf("last scancode: %d\n", last_scancode);
    if(last_scancode != 0)
    {
      // Any key was pressed or released
      break;
    }
  }

  // Start game
  busy_wait(250);
  run_menu();
}

void create_snake_element(uint16_t x, uint16_t y, uint8_t flags, uint8_t direction)
{
  struct snake_element *snake = (struct snake_element*) mm_alloc();
  if(snake == NULL)
  {
    printf("Allocation of %d Bytes failed.\n", sizeof(struct snake_element));
    return;
  }
  //printf("Created snake element at 0x%p\n", snake);


  snake->x = x;
  snake->y = y;
  snake->flags = flags;
  snake->direction = direction;

  //printf("New element x:%d y:%d flags:%d direction:%d\n", snake->x, snake->y, snake->flags, snake->direction);

  if(head == NULL)
  {
    head = snake;
    //printf("Attach as head\n");
  }
  else
  {
    //printf("Attach as body\n");

    // Go to the last element and add the node
    struct snake_element *ptr = head;
    while(ptr != NULL)
    {
      // Last element
      if(ptr->next == NULL)
      {
        //printf("Last element found. Attaching...\n");
        ptr->next = snake;
        break;
      }
      ptr = ptr->next;
    }
  }
}

void draw_snake(void)
{
  struct snake_element *ptr = head;
  //printf("Drawing snake\n");
  while(ptr != NULL)
  {
    //printf("Drawing element x:%d y:%d flags:%d direction:%d\n", ptr->x, ptr->y, ptr->flags, ptr->direction);
    if(ptr->flags & SNAKE_HEAD)
    {
      // Draw sprite rotated
      if(ptr->direction == SNAKE_FACING_NORTH)m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_head_0, 8, 8, ptr->x * 8, ptr->y * 8);
      else if(ptr->direction == SNAKE_FACING_EAST)m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_head_1, 8, 8, ptr->x * 8, ptr->y * 8);
      else if(ptr->direction == SNAKE_FACING_SOUTH)m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_head_2, 8, 8, ptr->x * 8, ptr->y * 8);
      else if(ptr->direction == SNAKE_FACING_WEST)m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_head_3, 8, 8, ptr->x * 8, ptr->y * 8);
    }
    else if (ptr->flags & SNAKE_DEFAULT)
    {
      if(ptr->direction == SNAKE_FACING_NORTH)m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_body_0, 8, 8, ptr->x * 8, ptr->y * 8);
      else if(ptr->direction == SNAKE_FACING_EAST)m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_body_1, 8, 8, ptr->x * 8, ptr->y * 8);
      else if(ptr->direction == SNAKE_FACING_SOUTH)m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_body_2, 8, 8, ptr->x * 8, ptr->y * 8);
      else if(ptr->direction == SNAKE_FACING_WEST)m13hb_draw_transparent_bitmap(screen_buffer, sprite_snake_body_3, 8, 8, ptr->x * 8, ptr->y * 8);
    }

    ptr = ptr->next;
  }
}

void translate_snake(void)
{
  struct snake_element *ptr = head;

  uint16_t prev_pos_x = ptr->x;
  uint16_t prev_pos_y = ptr->y;
  uint8_t prev_direction = ptr->direction;

  // Translate head
  if(snake_direction == SNAKE_DIRECTION_NORTH)ptr->y--;
  else if(snake_direction == SNAKE_DIRECTION_WEST)ptr->x--;
  else if(snake_direction == SNAKE_DIRECTION_SOUTH)ptr->y++;
  else if(snake_direction == SNAKE_DIRECTION_EAST)ptr->x++;

  // Change direction
  if(snake_direction == SNAKE_DIRECTION_NORTH)ptr->direction = SNAKE_FACING_NORTH;
  else if(snake_direction == SNAKE_DIRECTION_WEST)ptr->direction = SNAKE_FACING_WEST;
  else if(snake_direction == SNAKE_DIRECTION_SOUTH)ptr->direction = SNAKE_FACING_SOUTH;
  else if(snake_direction == SNAKE_DIRECTION_EAST)ptr->direction = SNAKE_FACING_EAST;

  ptr = ptr->next;

  // Translate body
  while(ptr != NULL)
  {
    uint16_t tmp_x = ptr->x;
    uint16_t tmp_y = ptr->y;
    uint8_t tmp_direction = ptr->direction;

    ptr->x = prev_pos_x;
    ptr->y = prev_pos_y;
    ptr->direction = prev_direction;

    prev_pos_x = tmp_x;
    prev_pos_y = tmp_y;
    prev_direction = tmp_direction;

    ptr = ptr->next;
  }
}

void spawn_food(void)
{
  //@TODO
  // World heigth -2 because last row is buggy on qemu
  food_pos_x = (uint16_t)maxrand(random_seed, WORLD_WIDTH - 1);
  food_pos_y = (uint16_t)maxrand(random_seed, WORLD_HEIGHT - 2);

  // Check if the food is inside the snake
  struct snake_element *ptr = head;
  while(ptr != NULL)
  {
    if((ptr->x == food_pos_x && ptr->y == food_pos_y)
        || food_pos_y == 0 || food_pos_y == 1)
    {
      food_pos_x = (uint16_t)maxrand(random_seed, WORLD_WIDTH - 1);
      food_pos_y = (uint16_t)maxrand(random_seed, WORLD_HEIGHT - 2);
      ptr = head;
    }
    else ptr = ptr->next;
  }

  printf("spawned food at: %dx%d\n", food_pos_x, food_pos_y);
}

unsigned long maxrand(unsigned long seed, unsigned long max)
{
	random_seed = random_seed + seed * 1103515245 + 12345;
	return (unsigned long)(random_seed / 65536) % (max + 1);
}

void busy_wait(uint32_t duration)
{
  uint32_t target = tick_counter + duration;
  while(tick_counter < target){}
}

void on_tick(void)
{
  tick_counter++;
}

void on_key(uint8_t scancode)
{
  //printf("read scancode: %x\n", scancode);
  last_scancode = scancode;
  // Change snake direction
  if(scancode == KEYBOARD_UP && snake_direction != SNAKE_DIRECTION_SOUTH)snake_direction = SNAKE_DIRECTION_NORTH;
  else if(scancode == KEYBOARD_DOWN && snake_direction != SNAKE_DIRECTION_NORTH)snake_direction = SNAKE_DIRECTION_SOUTH;
  else if(scancode == KEYBOARD_LEFT && snake_direction != SNAKE_DIRECTION_EAST)snake_direction = SNAKE_DIRECTION_WEST;
  else if(scancode == KEYBOARD_RIGHT && snake_direction != SNAKE_DIRECTION_WEST)snake_direction = SNAKE_DIRECTION_EAST;
}
