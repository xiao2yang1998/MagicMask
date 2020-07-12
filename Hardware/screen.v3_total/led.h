#ifndef __LED__H_
#define __LED__H_
#include "driverlib.h"
#endif

extern bool flag_light_on;
//extern float bright;
//extern float saturation;
//extern float hue;
void led_init(void);
void led_open_rgb(uint8_t red, uint8_t green, uint8_t blue);
void led_close(void);
void color_led_open_hsb(float hues, float saturation, float brightness);
