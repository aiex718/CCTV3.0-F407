#include "device/buzzer.h"


void Device_Buzzer_Init(Device_Buzzer_t *self)
{
    HAL_GPIO_InitPin(self->Buzzer_GPIO_pin);
    HAL_GPIO_WritePin(self->Buzzer_GPIO_pin,0);
    Buffer_Clear(self->Buzzer_Buffer);
    SysTimer_Init(&self->_buzzer_tmr,0);
}

void Device_Buzzer_Service(Device_Buzzer_t *self)
{
    uint16_t val;
    if(SysTimer_IsElapsed(&self->_buzzer_tmr))
    {
        if(Buffer_Queue_IsEmpty(self->Buzzer_Buffer))
            HAL_GPIO_WritePin(self->Buzzer_GPIO_pin,0);
        else if(Buffer_Queue_Pop_uint16_t(self->Buzzer_Buffer,&val) && val)
        {
            SysTimer_Init(&self->_buzzer_tmr,val);
            HAL_GPIO_TogglePin(self->Buzzer_GPIO_pin);
        }
    }
}

bool Device_Buzzer_DoBeep(Device_Buzzer_t *self,uint16_t duration,uint16_t wait)
{
    if(Buffer_Queue_GetCapacity(self->Buzzer_Buffer) < 2)
        return false;
    else if (duration==0||wait==0)
        return false;

    Buffer_Queue_Push_uint16_t(self->Buzzer_Buffer,duration);
    Buffer_Queue_Push_uint16_t(self->Buzzer_Buffer,wait);
    return true;
}


void Device_Buzzer_ShortBeep(Device_Buzzer_t *self)
{
    Device_Buzzer_DoBeep(self,100,100);
}

void Device_Buzzer_ShortDoubleBeep(Device_Buzzer_t *self)
{
    Device_Buzzer_DoBeep(self,50,50);
    Device_Buzzer_DoBeep(self,50,50);
}

void Device_Buzzer_LongBeep(Device_Buzzer_t *self)
{
    Device_Buzzer_DoBeep(self,500,200);
}

void Device_Buzzer_LongDoubleBeep(Device_Buzzer_t *self)
{
    Device_Buzzer_DoBeep(self,500,200);
    Device_Buzzer_DoBeep(self,500,200);
}
