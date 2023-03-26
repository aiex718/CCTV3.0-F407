#ifndef PERIPH_LIST_H
#define PERIPH_LIST_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/systick.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/rcc.h"
#include "bsp/hal/usart.h"

//LEDs active high
extern const HAL_GPIO_pin_t* LED_Load_pin;
extern const HAL_GPIO_pin_t* LED_STAT_pin;

//Buttons active high
extern const HAL_GPIO_pin_t* Button_Wkup_pin;

//Debug serial
#define Debug_Serial_Tx_Buffer_Size 256
#define Debug_Serial_Rx_Buffer_Size 16
extern HAL_USART_t *Debug_Usart3;


#endif
