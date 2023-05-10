#include "device/button.h"

static void Button_SendCallbackIfNotSent(Device_Button_t *self,Device_Button_CallbackIdx_t cb_idx);

void Device_Button_Init(Device_Button_t *self)
{
    HAL_GPIO_InitPin(self->Button_GPIO_pin);
    self->_button_prev_state = self->Button_IdleState;
    self->_button_callback_done_flags=0;
    SysTimer_Init(&self->_button_scan_tmr,self->Button_Scan_Period);
    BSP_ARR_CLEAR(self->Button_Callbacks);
}

void Device_Button_SetCallback(Device_Button_t* self, Device_Button_CallbackIdx_t cb_idx, Callback_t* callback)
{
    if(cb_idx < __NOT_CALLBACK_BUTTON_MAX)
        self->Button_Callbacks[cb_idx] = callback;

}

void Device_Button_Service(Device_Button_t *self)
{
    if(SysTimer_IsElapsed(&self->_button_scan_tmr))
    {
        SysTime_t now = SysTime_Get();
        bool new_state = HAL_GPIO_ReadPin(self->Button_GPIO_pin);

        if(new_state != self->_button_prev_state)
        {
            self->_button_prev_state = new_state;
            self->_button_active_timestamp = now;
            if(new_state == self->Button_IdleState)
            {
                self->_button_callback_done_flags=0;
                Callback_Invoke_Idx(self,NULL,self->Button_Callbacks,BUTTON_CALLBACK_RELEASE);
            }
            else
                Callback_Invoke_Idx(self,NULL,self->Button_Callbacks,BUTTON_CALLBACK_PRESS);

        }
        else if(self->_button_prev_state != self->Button_IdleState)
        {
            uint32_t diff = now - self->_button_active_timestamp;
            
            if(diff >= self->Button_ShortPress_Period)
                Button_SendCallbackIfNotSent(self,BUTTON_CALLBACK_SHORT_PRESS);
            
            if(diff >= self->Button_LongPress_Period)
                Button_SendCallbackIfNotSent(self,BUTTON_CALLBACK_LONG_PRESS);
                        
        }

        SysTimer_Reset(&self->_button_scan_tmr);
    }
}

static void Button_SendCallbackIfNotSent(Device_Button_t *self,Device_Button_CallbackIdx_t cb_idx)
{
    if(BitFlag_IsIdxSet(self->_button_callback_done_flags,cb_idx) == false)
    {
        Callback_Invoke_Idx(self,NULL,self->Button_Callbacks,cb_idx);
        BitFlag_SetIdx(self->_button_callback_done_flags,cb_idx);
    }
}
