#ifndef PERI_GPIO_H
#define PERI_GPIO_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"

//LEDs active high
extern const HAL_GPIO_pin_t *Periph_LED_Load_pin;
extern const HAL_GPIO_pin_t *Periph_LED_STAT_pin;

//Buttons active high
extern const HAL_GPIO_pin_t *Periph_Button_Wkup_pin;

#endif
