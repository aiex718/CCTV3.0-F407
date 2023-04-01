#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/timer_pwm.h"

#ifndef FLASHLIGHT_PWM_DUTYCYCLE_MAX 
    #define FLASHLIGHT_PWM_DUTYCYCLE_MAX 500 //cap duty cycle at 50%
#endif

typedef struct 
{
    HAL_Timer_PWM_t *FlashLight_Timer_PWM;
    HAL_Timer_PWM_Channel_t FlashLight_PWM_Channel;
    uint32_t FlashLight_PWM_DutyCycle;
}Device_FlashLight_t;

void Device_FlashLight_Attach(Device_FlashLight_t *self,HAL_Timer_PWM_t *timer_PWM);
void Device_FlashLight_Cmd(Device_FlashLight_t *self,bool en);
void Device_FlashLight_SetDutyCycle(Device_FlashLight_t *self,uint32_t duty_cycle);
uint32_t Device_FlashLight_GetDutyCycle(Device_FlashLight_t *self);

#endif
