#ifndef BUZZER_H
#define BUZZER_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/buffer.h"

typedef struct Device_Buzzer_s
{
    HAL_GPIO_pin_t *Buzzer_GPIO_pin;
    Buffer_uint16_t *Buzzer_Buffer;

    //private
    SysTimer_t _buzzer_tmr;
}Device_Buzzer_t;

__STATIC_INLINE bool Device_Buzzer_IsEmpty(Device_Buzzer_t *self)
{
    return Buffer_Queue_IsEmpty(self->Buzzer_Buffer);
}

void Device_Buzzer_Init(Device_Buzzer_t *self);
void Device_Buzzer_Service(Device_Buzzer_t *self);
bool Device_Buzzer_DoBeep(Device_Buzzer_t *self,uint16_t duration,uint16_t wait);

void Device_Buzzer_ShortBeep(Device_Buzzer_t *self);
void Device_Buzzer_ShortDoubleBeep(Device_Buzzer_t *self);
void Device_Buzzer_LongBeep(Device_Buzzer_t *self);
void Device_Buzzer_LongDoubleBeep(Device_Buzzer_t *self);



#endif
