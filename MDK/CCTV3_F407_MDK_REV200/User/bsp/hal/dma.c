#include "bsp/hal/dma.h"

static const uint32_t DMA_CBidx_to_ITConfigFlag_Tbl[__NOT_CALLBACK_DMA_MAX] = {
    DMA_IT_HT,
    DMA_IT_TC,
    DMA_IT_TE,
    DMA_IT_FE,
    DMA_IT_DME,
};

typedef struct
{
    uint32_t DMA_StreamOffset;
    // The DMA_ITStatusFlags order must be same as @ref HAL_DMA_CallbackIdx_t
    uint32_t DMA_ITStatusFlags[__NOT_CALLBACK_DMA_MAX];
    // DMA_IT_HTIFx
    // DMA_IT_TCIFx
    // DMA_IT_TEIFx
    // DMA_IT_FEIFx
    // DMA_IT_DMEIFx
}DMA_Stream_to_ITStatusFlags_Map_t;

const DMA_Stream_to_ITStatusFlags_Map_t DMA_Stream_to_ITStatusFlags_Tbl[] = {
    {DMA1_Stream0_BASE, {DMA_IT_HTIF0, DMA_IT_TCIF0, DMA_IT_TEIF0, DMA_IT_FEIF0, DMA_IT_DMEIF0}},
    {DMA1_Stream1_BASE, {DMA_IT_HTIF1, DMA_IT_TCIF1, DMA_IT_TEIF1, DMA_IT_FEIF1, DMA_IT_DMEIF1}},
    {DMA1_Stream2_BASE, {DMA_IT_HTIF2, DMA_IT_TCIF2, DMA_IT_TEIF2, DMA_IT_FEIF2, DMA_IT_DMEIF2}},
    {DMA1_Stream3_BASE, {DMA_IT_HTIF3, DMA_IT_TCIF3, DMA_IT_TEIF3, DMA_IT_FEIF3, DMA_IT_DMEIF3}},
    {DMA1_Stream4_BASE, {DMA_IT_HTIF4, DMA_IT_TCIF4, DMA_IT_TEIF4, DMA_IT_FEIF4, DMA_IT_DMEIF4}},
    {DMA1_Stream5_BASE, {DMA_IT_HTIF5, DMA_IT_TCIF5, DMA_IT_TEIF5, DMA_IT_FEIF5, DMA_IT_DMEIF5}},
    {DMA1_Stream6_BASE, {DMA_IT_HTIF6, DMA_IT_TCIF6, DMA_IT_TEIF6, DMA_IT_FEIF6, DMA_IT_DMEIF6}},
    {DMA1_Stream7_BASE, {DMA_IT_HTIF7, DMA_IT_TCIF7, DMA_IT_TEIF7, DMA_IT_FEIF7, DMA_IT_DMEIF7}},
    {DMA2_Stream0_BASE, {DMA_IT_HTIF0, DMA_IT_TCIF0, DMA_IT_TEIF0, DMA_IT_FEIF0, DMA_IT_DMEIF0}},
    {DMA2_Stream1_BASE, {DMA_IT_HTIF1, DMA_IT_TCIF1, DMA_IT_TEIF1, DMA_IT_FEIF1, DMA_IT_DMEIF1}},
    {DMA2_Stream2_BASE, {DMA_IT_HTIF2, DMA_IT_TCIF2, DMA_IT_TEIF2, DMA_IT_FEIF2, DMA_IT_DMEIF2}},
    {DMA2_Stream3_BASE, {DMA_IT_HTIF3, DMA_IT_TCIF3, DMA_IT_TEIF3, DMA_IT_FEIF3, DMA_IT_DMEIF3}},
    {DMA2_Stream4_BASE, {DMA_IT_HTIF4, DMA_IT_TCIF4, DMA_IT_TEIF4, DMA_IT_FEIF4, DMA_IT_DMEIF4}},
    {DMA2_Stream5_BASE, {DMA_IT_HTIF5, DMA_IT_TCIF5, DMA_IT_TEIF5, DMA_IT_FEIF5, DMA_IT_DMEIF5}},
    {DMA2_Stream6_BASE, {DMA_IT_HTIF6, DMA_IT_TCIF6, DMA_IT_TEIF6, DMA_IT_FEIF6, DMA_IT_DMEIF6}},
    {DMA2_Stream7_BASE, {DMA_IT_HTIF7, DMA_IT_TCIF7, DMA_IT_TEIF7, DMA_IT_FEIF7, DMA_IT_DMEIF7}},
};

//Public functions
void HAL_DMA_Init(HAL_DMA_t *self)
{
    uint8_t i;
    HAL_RCC_Cmd(self->DMA_RCC_Cmd, true);
    BSP_ARR_CLEAR(self->DMA_Callbacks);

    DMA_Init(self->DMA_Streamx, (DMA_InitTypeDef*)self->DMA_InitCfg);
    if(self->DMA_NVIC_InitCfg)
        NVIC_Init((NVIC_InitTypeDef*)self->DMA_NVIC_InitCfg);
    //assign it_status_flags to corresponding DMA_Streamx
    for(i = 0; i < BSP_ARR_LEN(DMA_Stream_to_ITStatusFlags_Tbl); i++)
    {
        if(DMA_Stream_to_ITStatusFlags_Tbl[i].DMA_StreamOffset == (uint32_t)self->DMA_Streamx)
        {
            self->__it_status_flags = (uint32_t*)DMA_Stream_to_ITStatusFlags_Tbl[i].DMA_ITStatusFlags;
            break;
        }
    }
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
    HAL_DMA_CallbackIdx_t cb_idx;

    DMA_DeInit(self->DMA_Streamx);
    while (DMA_GetCmdStatus(self->DMA_Streamx) != DISABLE);
    DMA_Init(self->DMA_Streamx, self->DMA_InitCfg);

    //reset all callbacks, enable corresponding interrupt
    for (cb_idx = (HAL_DMA_CallbackIdx_t)0; cb_idx < __NOT_CALLBACK_DMA_MAX; cb_idx++)
        HAL_DMA_SetCallback((HAL_DMA_t *)self, cb_idx, self->DMA_Callbacks[cb_idx]);
}

void HAL_DMA_SetCallback(HAL_DMA_t *self, HAL_DMA_CallbackIdx_t cb_idx, Callback_t* callback)
{
    if(cb_idx < __NOT_CALLBACK_DMA_MAX)
    {
        uint32_t ConfigFlag = DMA_CBidx_to_ITConfigFlag_Tbl[cb_idx];
        uint32_t StatusFlag = self->__it_status_flags[cb_idx];
        DMA_ClearITPendingBit(self->DMA_Streamx, StatusFlag);

        DMA_ITConfig(self->DMA_Streamx, ConfigFlag, callback?ENABLE:DISABLE);
        self->DMA_Callbacks[cb_idx] = callback;
    }
}


void HAL_DMA_IRQHandler(HAL_DMA_t *dma)
{
    HAL_DMA_CallbackIdx_t cb_idx;
    for(cb_idx = (HAL_DMA_CallbackIdx_t)0; cb_idx < __NOT_CALLBACK_DMA_MAX; cb_idx++)
    {
        uint32_t IT_StatusFlag = dma->__it_status_flags[cb_idx];
        if(DMA_GetITStatus(dma->DMA_Streamx, IT_StatusFlag))
        {
            DMA_ClearITPendingBit(dma->DMA_Streamx, IT_StatusFlag);
            Callback_Invoke_Idx(dma,NULL,dma->DMA_Callbacks, cb_idx);
        }
    }
}

