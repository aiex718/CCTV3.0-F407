#include "bsp/hal/dma.h"

void HAL_DMA_Init(const HAL_DMA_t *self)
{
    HAL_RCC_Cmd(self->DMA_RCC_Cmd, true);
    DMA_Init(self->DMA_Streamx, (DMA_InitTypeDef*)self->DMA_InitCfg);
    if(self->DMA_NVIC_InitCfg)
        NVIC_Init((NVIC_InitTypeDef*)self->DMA_NVIC_InitCfg);
}

void HAL_DMA_DeInit(const HAL_DMA_t *self)
{
    DMA_DeInit(self->DMA_Streamx);
    while (DMA_GetCmdStatus(self->DMA_Streamx) != DISABLE);
    HAL_RCC_Cmd(self->DMA_RCC_Cmd, false);
}

void HAL_DMA_Cmd(const HAL_DMA_t *self, bool en)
{
    if(en)
        DMA_Cmd(self->DMA_Streamx, ENABLE);
    else
    {
        DMA_Cmd(self->DMA_Streamx, DISABLE);
        while (DMA_GetCmdStatus(self->DMA_Streamx) != DISABLE);
    }
}

