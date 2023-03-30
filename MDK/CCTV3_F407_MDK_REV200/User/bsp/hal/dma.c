#include "bsp/hal/dma.h"

const uint32_t DMA_CB_to_ITConfigFlag_Tbl[__NOT_CALLBACK_DMA_MAX] = {
    DMA_IT_HT,
    DMA_IT_TC,
    DMA_IT_TE,
    DMA_IT_FE,
    DMA_IT_DME,
};

typedef struct
{
    uint16_t DMA_StreamOffset;
    uint32_t* DMA_ITClearFlag;
    // DMA_IT_HTIFx
    // DMA_IT_TCIFx
    // DMA_IT_TEIFx
    // DMA_IT_FEIFx
    // DMA_IT_DMEIFx
}DMA_ITtoFlags_Map_t;

//we use offset of DMA_Streamx as index, so that DMA1 and DMA2 can share the same array
const DMA_ITtoFlags_Map_t DMA_StreamOffset_to_ITClearFlag_Tbl[] ={
    {(uint16_t)(DMA1_Stream0_BASE - DMA1_BASE), (uint32_t*)&(const uint32_t[]){DMA_IT_HTIF0, DMA_IT_TCIF0, DMA_IT_TEIF0, DMA_IT_FEIF0, DMA_IT_DMEIF0}},
    {(uint16_t)(DMA1_Stream1_BASE - DMA1_BASE), (uint32_t*)&(const uint32_t[]){DMA_IT_HTIF1, DMA_IT_TCIF1, DMA_IT_TEIF1, DMA_IT_FEIF1, DMA_IT_DMEIF1}},
    {(uint16_t)(DMA1_Stream2_BASE - DMA1_BASE), (uint32_t*)&(const uint32_t[]){DMA_IT_HTIF2, DMA_IT_TCIF2, DMA_IT_TEIF2, DMA_IT_FEIF2, DMA_IT_DMEIF2}},
    {(uint16_t)(DMA1_Stream3_BASE - DMA1_BASE), (uint32_t*)&(const uint32_t[]){DMA_IT_HTIF3, DMA_IT_TCIF3, DMA_IT_TEIF3, DMA_IT_FEIF3, DMA_IT_DMEIF3}},
    {(uint16_t)(DMA1_Stream4_BASE - DMA1_BASE), (uint32_t*)&(const uint32_t[]){DMA_IT_HTIF4, DMA_IT_TCIF4, DMA_IT_TEIF4, DMA_IT_FEIF4, DMA_IT_DMEIF4}},
    {(uint16_t)(DMA1_Stream5_BASE - DMA1_BASE), (uint32_t*)&(const uint32_t[]){DMA_IT_HTIF5, DMA_IT_TCIF5, DMA_IT_TEIF5, DMA_IT_FEIF5, DMA_IT_DMEIF5}},
    {(uint16_t)(DMA1_Stream6_BASE - DMA1_BASE), (uint32_t*)&(const uint32_t[]){DMA_IT_HTIF6, DMA_IT_TCIF6, DMA_IT_TEIF6, DMA_IT_FEIF6, DMA_IT_DMEIF6}},
    {(uint16_t)(DMA1_Stream7_BASE - DMA1_BASE), (uint32_t*)&(const uint32_t[]){DMA_IT_HTIF7, DMA_IT_TCIF7, DMA_IT_TEIF7, DMA_IT_FEIF7, DMA_IT_DMEIF7}},
};

//Private helper functions 
static uint32_t __Get_ITClearFlag(DMA_Stream_TypeDef *DMA_Streamx, HAL_DMA_CallbackIdx_t cb_idx)
{
    uint8_t i;          
    uint32_t offset; 
    //calculate the offset of DMA_Streamx
    if((uint32_t)DMA_Streamx> DMA2_BASE)
        offset = ((uint32_t)DMA_Streamx - DMA2_BASE);
    else
        offset = ((uint32_t)DMA_Streamx - DMA1_BASE);
    //loop through the array to find the offset
    for(i = 0; i < BSP_ARRLEN(DMA_StreamOffset_to_ITClearFlag_Tbl); i++)
    {
        if(DMA_StreamOffset_to_ITClearFlag_Tbl[i].DMA_StreamOffset == offset)
            return DMA_StreamOffset_to_ITClearFlag_Tbl[i].DMA_ITClearFlag[cb_idx];
    }
    return 0;
}

//Public functions
void HAL_DMA_Init(HAL_DMA_t *self)
{
    HAL_RCC_Cmd(self->DMA_RCC_Cmd, true);
    BSP_MEMSET(self->DMA_Callbacks,0,sizeof(self->DMA_Callbacks));

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

void HAL_DMA_ReloadCfg(const HAL_DMA_t *self)
{
    DMA_DeInit(self->DMA_Streamx);
    while (DMA_GetCmdStatus(self->DMA_Streamx) != DISABLE);
    DMA_Init(self->DMA_Streamx, self->DMA_InitCfg);
}

void HAL_DMA_SetCallback(HAL_DMA_t *self, HAL_DMA_CallbackIdx_t cb_idx, Callback_t* callback)
{
    if(cb_idx < __NOT_CALLBACK_DMA_MAX)
    {
        uint32_t ConfigFlag = DMA_CB_to_ITConfigFlag_Tbl[cb_idx];
        uint32_t ClearFlag = __Get_ITClearFlag(self->DMA_Streamx, cb_idx);
        if(ClearFlag)
            DMA_ClearITPendingBit(self->DMA_Streamx, ClearFlag);

        DMA_ITConfig(self->DMA_Streamx, ConfigFlag, callback?ENABLE:DISABLE);
        self->DMA_Callbacks[cb_idx] = callback;
    }
}


void HAL_DMA_IRQHandler(HAL_DMA_t *dma)
{
    HAL_DMA_CallbackIdx_t cb_idx;
    for(cb_idx = (HAL_DMA_CallbackIdx_t)0; cb_idx < __NOT_CALLBACK_DMA_MAX; cb_idx++)
    {
        uint32_t IT_ConfigFlag = DMA_CB_to_ITConfigFlag_Tbl[cb_idx];
        if(DMA_GetITStatus(dma->DMA_Streamx, IT_ConfigFlag))
        {
            uint32_t IT_ClearFlag = __Get_ITClearFlag(dma->DMA_Streamx, cb_idx);
            DMA_ClearITPendingBit(dma->DMA_Streamx, IT_ClearFlag);
            Callback_InvokeIdx(dma,dma->DMA_Callbacks, cb_idx);
        }
    }
}

