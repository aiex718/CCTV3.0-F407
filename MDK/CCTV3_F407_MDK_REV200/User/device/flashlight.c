#include "device/flashlight.h"

void Device_FlashLight_Init(Device_FlashLight_t *self)
{
    HAL_GPIO_InitPin(self->FlashLight_GPIO_pin);
    if(self->FlashLight_Timer_PWM)
    {
        HAL_Timer_PWM_InitChannel(self->FlashLight_Timer_PWM,
        self->FlashLight_Timer_PWM_Channel);
        Device_FlashLight_SetBrightness(self,self->FlashLight_Brightness);
    }
}

void Device_FlashLight_Attach(Device_FlashLight_t *self,const HAL_Timer_PWM_t *timer_PWM)
{
    self->FlashLight_Timer_PWM = timer_PWM;
}

void Device_FlashLight_Cmd(Device_FlashLight_t *self,bool en)
{
    if(self->FlashLight_Timer_PWM)
    {
        HAL_Timer_PWM_ChannelCmd(self->FlashLight_Timer_PWM,
            self->FlashLight_Timer_PWM_Channel,en);
    }
}

void Device_FlashLight_SetBrightness(Device_FlashLight_t *self,uint8_t brightness)
{
    if(self->FlashLight_Timer_PWM)
    {
        uint32_t period = HAL_Timer_PWM_GetPeriod(self->FlashLight_Timer_PWM);
        self->FlashLight_Brightness = BSP_MIN(brightness,FLASHLIGHT_BRIGHTNESS_MAX);
        HAL_Timer_PWM_SetDutyCycle(self->FlashLight_Timer_PWM,
            self->FlashLight_Timer_PWM_Channel,(period * self->FlashLight_Brightness) / 100);
    }
}

uint8_t Device_FlashLight_GetBrightness(Device_FlashLight_t *self)
{
    return self->FlashLight_Brightness;
}
