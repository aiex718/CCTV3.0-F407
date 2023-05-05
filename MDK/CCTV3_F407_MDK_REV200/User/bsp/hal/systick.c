#include "bsp/hal/systick.h"

void HAL_Systick_Init(void)
{
    RCC_ClocksTypeDef rcc_clocks;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    RCC_GetClocksFreq(&rcc_clocks);
    SysTick_Config((rcc_clocks.HCLK_Frequency / 1000 ));
    NVIC_SetPriority(SysTick_IRQn, 0);
}




