#include "bsp/hal/systick.h"

static __IO uint32_t systime_tick;

void HAL_Systick_Init(void)
{
    RCC_ClocksTypeDef rcc_clocks;
    systime_tick=0;
    RCC_GetClocksFreq(&rcc_clocks);
    SysTick_Config((rcc_clocks.HCLK_Frequency / 1000 )-1);
}




