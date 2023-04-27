#ifndef PERIPH_LIST_H
#define PERIPH_LIST_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/usart.h"

//LEDs active high
extern const HAL_GPIO_pin_t *Periph_LED_Load_pin;
extern const HAL_GPIO_pin_t *Periph_LED_STAT_pin;

//Buttons active high
extern const HAL_GPIO_pin_t *Periph_Button_Wkup_pin;

//Debug serial
extern DBG_Serial_t* DBG_Serial;

#endif
