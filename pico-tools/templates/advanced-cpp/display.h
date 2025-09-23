#ifndef DISPLAY_H
#define DISPLAY_H

#include "pico/stdlib.h"

// Display interface
void display_init();
void display_update_demo(float temperature, uint16_t light_level);
void display_clear();
void display_print(const char* text);
void display_set_cursor(uint8_t x, uint8_t y);

#endif // DISPLAY_H