#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/gpio.h"
#include "bsp/platform/periph_list.h"

__STATIC_INLINE void Default_SysFault_Loop(const char *msg,uint16_t blink_period,uint16_t print_period)
{
    SysTimer_t tmr_blink,tmr_print;
    SysTimer_Init(&tmr_blink,blink_period);
    SysTimer_Init(&tmr_print,print_period);
    
    DBG_Serial_SafeMode(DBG_Serial,true);
    printf("%s",msg);
    
    while (1)
    {
        if(SysTimer_IsElapsed(&tmr_blink))
        {
            SysTimer_Reset(&tmr_blink);
            HAL_GPIO_TogglePin(Periph_LED_STAT_pin);
        }
        if(SysTimer_IsElapsed(&tmr_print))
        {
            SysTimer_Reset(&tmr_print);
            printf("%s",msg);
        }
    }
}

void NMI_Handler(void)
{
    Default_SysFault_Loop("NMI_Handler",1000,3000);
}

void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    Default_SysFault_Loop("HardFault_Handler",200,3000);
}

void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    Default_SysFault_Loop("MemManage_Handler",200,3000);
}

void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    Default_SysFault_Loop("BusFault_Handler",200,3000);
}

void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    Default_SysFault_Loop("UsageFault_Handler",200,3000);
}

void DebugMon_Handler(void)
{

}

void SVC_Handler(void)
{

}

void PendSV_Handler(void)
{

}

void SysTick_Handler(void)
{
    SysTime_Inc();
}

