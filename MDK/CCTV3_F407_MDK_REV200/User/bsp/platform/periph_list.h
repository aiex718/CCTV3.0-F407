#ifndef PERIPH_LIST_H
#define PERIPH_LIST_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/usart.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"
#include "device/flashlight.h"


//LEDs active high
extern const HAL_GPIO_pin_t *LED_Load_pin;
extern const HAL_GPIO_pin_t *LED_STAT_pin;

//Buttons active high
extern const HAL_GPIO_pin_t *Button_Wkup_pin;

//Debug serial
#define Debug_Serial_Tx_Buffer_Size 512
#define Debug_Serial_Rx_Buffer_Size 16
extern HAL_USART_t *Debug_Usart3;
extern DBG_Serial_t *DBG_Serial;

//Timer PWM
extern HAL_Timer_PWM_t *Timer_PWM_FlashLight;

//Device flashlight
extern Device_FlashLight_t *FlashLight_Top;
extern Device_FlashLight_t *FlashLight_Bottom;


//TODO: MCO 



#endif
