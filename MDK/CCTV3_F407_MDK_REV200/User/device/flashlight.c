#include "device/flashlight.h"

void Device_FlashLight_Attach(Device_FlashLight_t *self,HAL_Timer_PWM_t *timer_PWM)
{
    self->FlashLight_Timer_PWM = timer_PWM;
    HAL_Timer_PWM_SetDutyCycle(self->FlashLight_Timer_PWM,self->FlashLight_PWM_Channel,self->FlashLight_PWM_DutyCycle);
}

void Device_FlashLight_Cmd(Device_FlashLight_t *self,bool en)
{
    if(en)
        Device_FlashLight_SetDutyCycle(self,self->FlashLight_PWM_DutyCycle);
    else
        Device_FlashLight_SetDutyCycle(self,0);
    
    HAL_Timer_PWM_Cmd(self->FlashLight_Timer_PWM,en);
}

void Device_FlashLight_SetDutyCycle(Device_FlashLight_t *self,uint32_t duty_cycle)
{
    HAL_Timer_PWM_SetDutyCycle( self->FlashLight_Timer_PWM,
        self->FlashLight_PWM_Channel,BSP_MIN(duty_cycle,
        FLASHLIGHT_PWM_DUTYCYCLE_MAX));
}

uint32_t Device_FlashLight_GetDutyCycle(Device_FlashLight_t *self)
{
    return HAL_Timer_PWM_GetDutyCycle(self->FlashLight_Timer_PWM,
        self->FlashLight_PWM_Channel);
}
