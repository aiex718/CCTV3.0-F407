#ifndef PERI_GPIO_H
#define PERI_GPIO_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"

//LEDs active high
extern const HAL_GPIO_pin_t Peri_LED_STAT_pin_Inst;
extern const HAL_GPIO_pin_t *Peri_LED_STAT_pin;

#endif
