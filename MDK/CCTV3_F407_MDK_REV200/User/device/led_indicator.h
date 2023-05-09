#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"
#include "bsp/sys/systimer.h"

typedef struct Device_LedIndicator_s
{
    HAL_GPIO_pin_t *LedIndicator_GPIO_pin;
    uint32_t LedIndicator_Timer_Period;
    //private
    SysTimer_t _led_indi_tmr;
}Device_LedIndicator_t;

void Device_LedIndicator_Init(Device_LedIndicator_t *self);
void Device_LedIndicator_Service(Device_LedIndicator_t *self);


#endif
