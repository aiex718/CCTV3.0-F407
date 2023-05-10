#ifndef BUTTON_H
#define BUTTON_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/bitflag.h"
#include "bsp/sys/callback.h"

typedef enum 
{
    //Button callbacks always invoked in service function
    BUTTON_CALLBACK_PRESS  = 0      ,
    BUTTON_CALLBACK_RELEASE         ,
    BUTTON_CALLBACK_SHORT_PRESS     ,
    BUTTON_CALLBACK_LONG_PRESS      ,
    __NOT_CALLBACK_BUTTON_MAX       ,
}Device_Button_CallbackIdx_t;

typedef struct Device_Button_s
{
    HAL_GPIO_pin_t *Button_GPIO_pin;
    uint32_t Button_LongPress_Period;
    uint32_t Button_ShortPress_Period;
    Callback_t* Button_Callbacks[__NOT_CALLBACK_BUTTON_MAX];
    uint16_t Button_Scan_Period;
    bool Button_IdleState;
    //private
    bool _button_prev_state;
    SysTime_t _button_active_timestamp;
    SysTimer_t _button_scan_tmr;
    BitFlag8_t _button_callback_done_flags;//prevent callback invoke more than once
}Device_Button_t;

__STATIC_INLINE bool Device_Button_IsPressed(Device_Button_t *self)
{
    return HAL_GPIO_ReadPin(self->Button_GPIO_pin) != self->Button_IdleState;
}

void Device_Button_Init(Device_Button_t *self);
void Device_Button_SetCallback(Device_Button_t* self, Device_Button_CallbackIdx_t cb_idx, Callback_t* callback);
void Device_Button_Service(Device_Button_t *self);


#endif
