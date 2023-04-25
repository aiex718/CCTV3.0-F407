#ifndef PERIPH_LIST_H
#define PERIPH_LIST_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/usart.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"
#include "bsp/hal/mco.h"
#include "bsp/hal/uniqueid.h"

#include "device/flashlight.h"
#include "device/cam_ov2640/cam_ov2640.h"

#include "eth/apps/mjpeg/mjpegd.h"


//LEDs active high
extern HAL_GPIO_pin_t *Periph_LED_Load_pin;
extern HAL_GPIO_pin_t *Periph_LED_STAT_pin;

//Buttons active high
extern HAL_GPIO_pin_t *Periph_Button_Wkup_pin;

//Debug serial
#define DEBUG_USART3_TX_BUFFER_SIZE 256
#define DEBUG_USART3_RX_BUFFER_SIZE 16
#define DEBUG_SERIAL_TX_BUFFER_SIZE 1024
#define DEBUG_SERIAL_RX_BUFFER_SIZE 32
extern DBG_Serial_t *DBG_Serial;

//Timer PWM
extern HAL_Timer_PWM_t *Periph_Timer_PWM_FlashLight;

//Device flashlight
extern Device_FlashLight_t *Periph_FlashLight_Top;
extern Device_FlashLight_t *Periph_FlashLight_Bottom;

//Unique ID
extern HAL_UniqueID_t *Periph_UniqueID; 

//Camera
extern HAL_MCO_t *Periph_MCO2_Cam;//MCO2 for camera
extern Device_CamOV2640_t* Periph_Cam_OV2640;

//Mjpegd
extern Mjpegd_t* Periph_Mjpegd;


#endif
