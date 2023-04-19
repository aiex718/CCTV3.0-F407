#include "bsp/hal/dcmi.h"
#include "bsp/hal/dma.h"

static const uint16_t DCMI_CBidx_to_ITFlag_Tbl[__NOT_CALLBACK_DCMI_MAX] = {
    DCMI_IT_FRAME   ,
    DCMI_IT_OVF     ,
    DCMI_IT_ERR     ,
    DCMI_IT_VSYNC   ,
    DCMI_IT_LINE    ,
};

void HAL_DCMI_Init(HAL_DCMI_t* self)
{
    HAL_GPIO_pin_t **pin = self->DCMI_Pins;
    //todo: check all cfg before init
    HAL_RCC_Cmd(self->DCMI_RCC_Cmd, ENABLE);
    while(*pin)
        HAL_GPIO_InitPin(*pin++);
    if(self->DCMI_InitCfg)
        DCMI_Init(self->DCMI_InitCfg);
    if(self->DCMI_NVIC_InitCfg)
        NVIC_Init(self->DCMI_NVIC_InitCfg);
    if(self->DCMI_RxDma_Cfg)
        HAL_DMA_Init(self->DCMI_RxDma_Cfg);
        
    BSP_ARR_CLEAR(self->DCMI_Callbacks);
    self->pExtension = NULL;
    self->_callback_pending_flag = 0;
}

void HAL_DCMI_SetCallback(HAL_DCMI_t* self, 
    HAL_DCMI_CallbackIdx_t cb_idx, Callback_t* callback)
{
    if(cb_idx < __NOT_CALLBACK_DCMI_MAX)
    {
        uint16_t ITFlag = DCMI_CBidx_to_ITFlag_Tbl[cb_idx];
        DCMI_ClearITPendingBit(ITFlag);

        DCMI_ITConfig(ITFlag, callback?ENABLE:DISABLE);
        self->DCMI_Callbacks[cb_idx] = callback;
    }
}

HAL_DCMI_Status_t HAL_DCMI_StartDmaRecv(HAL_DCMI_t* self, uint8_t* buf, uint32_t len)
{
    const HAL_DMA_t *dma_cfg = self->DCMI_RxDma_Cfg;
    if(dma_cfg==NULL || buf==NULL || len==0)
        return HAL_DCMI_BAD_ARGS;
    //HAL_DMA_ReloadCfg(dma_cfg);
    HAL_DMA_SetMemAddr(dma_cfg, buf);
    HAL_DMA_SetPeriphAddr(dma_cfg , &(DCMI->DR));
    HAL_DMA_SetNumOfData(dma_cfg, len>>2);//div 4 because DCMI bus is 32bit
    HAL_DMA_Cmd(dma_cfg,true);

    return HAL_DCMI_OK;
}

void HAL_DCMI_JpegCmd(HAL_DCMI_t* self, bool en)
{
    DCMI_JPEGCmd(en?ENABLE:DISABLE);
}

void HAL_DCMI_CaptureCmd(HAL_DCMI_t* self, bool en)
{
    DCMI_CaptureCmd(en?ENABLE:DISABLE);
}

void HAL_DCMI_Cmd(HAL_DCMI_t* self, bool en)
{
    DCMI_Cmd(en?ENABLE:DISABLE);
}

void HAL_DCMI_IRQHandler(HAL_DCMI_t* self)
{
    HAL_DCMI_CallbackIdx_t cb_idx;
    for(cb_idx = (HAL_DCMI_CallbackIdx_t)0; 
        cb_idx < __NOT_CALLBACK_DCMI_MAX; 
        cb_idx++)
    {
        uint16_t ITFlag = DCMI_CBidx_to_ITFlag_Tbl[cb_idx];
        if(DCMI_GetITStatus(ITFlag))
        {
            DCMI_ClearITPendingBit(ITFlag);
            Callback_InvokeNowOrPending_Idx(self, NULL,
                self->DCMI_Callbacks, cb_idx,self->_callback_pending_flag);
        }
    }
}

void HAL_DCMI_Service(HAL_DCMI_t* self)
{
    //execute pending flags
    while(self->_callback_pending_flag)
    {
        uint8_t cb_idx = BitFlag_BinToIdx(self->_callback_pending_flag);
        Callback_Invoke_Idx(self,NULL,self->DCMI_Callbacks,cb_idx);        
        BitFlag_ClearIdx(self->_callback_pending_flag,cb_idx);
    }
}
