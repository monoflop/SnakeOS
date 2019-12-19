#ifndef SNAKE_H
#define SNAKE_H

#include "stdint.h"

void snake_init(void);
void on_tick(void);
void on_key(uint8_t scancode);

#endif
