#include "bsp/platform/platform_inst.h"

//periph interrupt handlers
void USART3_IRQHandler(void)
{
    HAL_USART_IRQHandler(Peri_DBG_Serial->hal_usart);
}

void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Dev_CurrentTrig->CurrentTrig_DMA);
}
