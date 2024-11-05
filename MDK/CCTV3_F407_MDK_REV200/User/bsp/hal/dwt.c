#include "bsp/hal/dwt.h"

void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT_Clear();
}

uint32_t DWT_GetCycle(void)
{
    return DWT->CYCCNT;
}

void DWT_Clear(void)
{
    DWT->CYCCNT = 0;
}


