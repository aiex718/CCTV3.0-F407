#include "bsp/sys/systime.h"
#include "bsp/platform/periph_list.h"

void NMI_Handler(void)
{

}

void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {}
}

void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {}
}

void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {}
}

void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {}
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
    Systime_Inc();
}


void USART3_IRQHandler(void)
{
	HAL_USART_IRQHandler(Debug_Usart3);
}
