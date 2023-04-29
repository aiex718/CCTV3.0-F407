#include "bsp/sys/systime.h"
#include "bsp/sys/dbg_serial.h"

__STATIC_INLINE void Default_SysFault_Loop(const char *msg,uint32_t delay)
{
    DBG_Serial_SafeMode(Peri_DBG_Serial,true);
    __IO uint32_t cnt;
    
    while (1)
    {
        cnt=delay;
        printf("%s",msg);
        while(cnt--);
    }
}

void NMI_Handler(void)
{
    Default_SysFault_Loop("NMI_Handler\n",0xfffff);
}

void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    Default_SysFault_Loop("HardFault_Handler\n",0xfffff);
}

void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    Default_SysFault_Loop("MemManage_Handler\n",0xfffff);
}

void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    Default_SysFault_Loop("BusFault_Handler\n",0xfffff);
}

void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    Default_SysFault_Loop("UsageFault_Handler\n",0xfffff);
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

