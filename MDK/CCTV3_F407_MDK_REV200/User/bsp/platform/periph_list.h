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
#define DEBUG_USART3_TX_BUFFER_SIZE 256
#define DEBUG_USART3_RX_BUFFER_SIZE 16
#define DEBUG_SERIAL_TX_BUFFER_SIZE 1024
#define DEBUG_SERIAL_RX_BUFFER_SIZE 32
extern DBG_Serial_t* DBG_Serial;

#endif
