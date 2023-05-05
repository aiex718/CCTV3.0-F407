#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/timer_pwm.h"
#include "bsp/hal/gpio.h"

#ifndef FLASHLIGHT_BRIGHTNESS_MAX 
    #define FLASHLIGHT_BRIGHTNESS_MAX 50 //cap duty cycle at 50%
#endif

typedef struct Device_FlashLight_s
{
    const HAL_Timer_PWM_t *FlashLight_Timer_PWM; //not allow to modify parent timer
    HAL_Timer_PWM_Channel_t *FlashLight_Timer_PWM_Channel;
    HAL_GPIO_pin_t *FlashLight_GPIO_pin;
    uint8_t FlashLight_Brightness;
}Device_FlashLight_t;

void Device_FlashLight_Init(Device_FlashLight_t *self);
void Device_FlashLight_Attach_PWM(Device_FlashLight_t *self,const HAL_Timer_PWM_t *timer_PWM);
void Device_FlashLight_Cmd(Device_FlashLight_t *self,bool en);
void Device_FlashLight_SetBrightness(Device_FlashLight_t *self,uint8_t brightness);
uint8_t Device_FlashLight_GetBrightness(Device_FlashLight_t *self);

#endif
