#include "bsp/hal/dma.h"

void HAL_DMA_Init(const HAL_DMA_t *self)
{
    HAL_RCC_Cmd(self->DMA_RCC_Cmd, true);
    DMA_Init(self->DMA_Streamx, (DMA_InitTypeDef*)self->DMA_InitCfg);
    NVIC_Init((NVIC_InitTypeDef*)self->DMA_NVIC_InitCfg);
}

