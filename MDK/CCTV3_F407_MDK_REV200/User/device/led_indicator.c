#include "device/led_indicator.h"

void Device_LedIndicator_Init(Device_LedIndicator_t *self)
{
    HAL_GPIO_InitPin(self->LedIndicator_GPIO_pin);
    HAL_GPIO_WritePin(self->LedIndicator_GPIO_pin,false);
    SysTimer_Init(&self->_led_indi_tmr,self->LedIndicator_Timer_Period);
}

void Device_LedIndicator_Service(Device_LedIndicator_t *self)
{
    if(SysTimer_IsElapsed(&self->_led_indi_tmr))
    {
        HAL_GPIO_TogglePin(self->LedIndicator_GPIO_pin);
        SysTimer_Reset(&self->_led_indi_tmr);
    }
}
