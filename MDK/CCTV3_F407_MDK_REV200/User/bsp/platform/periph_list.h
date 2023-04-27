#ifndef PERIPH_LIST_H
#define PERIPH_LIST_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/usart.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"
#include "bsp/hal/rng.h"
#include "bsp/hal/uniqueid.h"
#include "bsp/hal/rtc.h"

#include "device/flashlight.h"
#include "device/cam_ov2640/cam_ov2640.h"

#include "eth/apps/mjpeg/mjpegd.h"
#include "eth/apps/nettime/nettime.h"


//RCC
extern HAL_RCC_t *Periph_RCC;

//RTC
extern HAL_RTC_t *Periph_RTC;

//LEDs active high
extern HAL_GPIO_pin_t *Periph_LED_Load_pin;
extern HAL_GPIO_pin_t *Periph_LED_STAT_pin;

//Buttons active high
extern HAL_GPIO_pin_t *Periph_Button_Wkup_pin;

//Debug serial
extern DBG_Serial_t *DBG_Serial;

//RNG
extern HAL_Rng_t *Periph_Rng;

//Timer PWM
extern HAL_Timer_PWM_t *Periph_Timer_PWM_FlashLight;

//Device flashlight
extern Device_FlashLight_t *Periph_FlashLight_Top;
extern Device_FlashLight_t *Periph_FlashLight_Bottom;

//Unique ID
extern HAL_UniqueID_t *Periph_UniqueID; 

//Camera
extern Device_CamOV2640_t* Periph_Cam_OV2640;

//Mjpegd
extern Mjpegd_t* APPs_Mjpegd;

extern NetTime_t* APPs_NetTime;

#endif
