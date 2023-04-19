#ifndef PERIPH_LIST_H
#define PERIPH_LIST_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/usart.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"
#include "bsp/hal/mco.h"
#include "device/flashlight.h"
#include "device/cam_ov2640/cam_ov2640.h"


//LEDs active high
extern HAL_GPIO_pin_t *LED_Load_pin;
extern HAL_GPIO_pin_t *LED_STAT_pin;

//Buttons active high
extern HAL_GPIO_pin_t *Button_Wkup_pin;

//Debug serial
#define DEBUG_USART3_TX_BUFFER_SIZE 256
#define DEBUG_USART3_RX_BUFFER_SIZE 16
#define DEBUG_SERIAL_TX_BUFFER_SIZE 1024
#define DEBUG_SERIAL_RX_BUFFER_SIZE 32
extern DBG_Serial_t *DBG_Serial;

//Timer PWM
extern HAL_Timer_PWM_t *Timer_PWM_FlashLight;

//Device flashlight
extern Device_FlashLight_t *FlashLight_Top;
extern Device_FlashLight_t *FlashLight_Bottom;

//Camera
extern HAL_MCO_t *MCO2_Cam;//MCO2 for camera
extern Device_CamOV2640_t* Cam_OV2640;




#endif
