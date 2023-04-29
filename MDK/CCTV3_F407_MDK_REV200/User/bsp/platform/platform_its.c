#include "bsp/platform/platform_inst.h"

//periph interrupt handlers
void USART3_IRQHandler(void)
{
    HAL_USART_IRQHandler(Peri_DBG_Serial->hal_usart);
}

void DCMI_IRQHandler(void)
{
    HAL_DCMI_IRQHandler(Dev_Cam_OV2640->CamOV2640_DCMI);
}

void DMA2_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Dev_Cam_OV2640->CamOV2640_DCMI->DCMI_RxDma_Cfg);
}

