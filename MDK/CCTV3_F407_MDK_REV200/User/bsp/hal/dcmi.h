#ifndef DCMI_H
#define DCMI_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/callback.h"
#include "bsp/sys/bitflag.h"

#include "bsp/hal/gpio.h"
#include "bsp/hal/rcc.h"
#include "bsp/hal/dma.h"

typedef enum 
{
    HAL_DCMI_OK =0         ,
    HAL_DCMI_ERROR         ,
    HAL_DCMI_BUSY          ,
    HAL_DCMI_BAD_ARGS      ,
}HAL_DCMI_Status_t;

typedef enum 
{
    DCMI_CALLBACK_FRAME = 0     ,
    DCMI_CALLBACK_OVF           ,
    DCMI_CALLBACK_ERR           ,
    DCMI_CALLBACK_VSYNC         ,
    DCMI_CALLBACK_LINE          ,
    __NOT_CALLBACK_DCMI_MAX     ,
}HAL_DCMI_CallbackIdx_t;

typedef struct HAL_DCMI_s
{
    DCMI_TypeDef* DCMIx;
    HAL_RCC_Cmd_t* DCMI_RCC_Cmd;
    HAL_GPIO_pin_t** DCMI_Pins;
    DCMI_InitTypeDef* DCMI_InitCfg;
    //nvic
    NVIC_InitTypeDef* DCMI_NVIC_InitCfg;
    //dma cfg
    HAL_DMA_t* DCMI_RxDma_Cfg;
    Callback_t* DCMI_Callbacks[__NOT_CALLBACK_DCMI_MAX];
    //customize data structure
    void* pExtension;
    //private flags, dont use
    BitFlag_t _callback_pending_flag;
}HAL_DCMI_t;

void HAL_DCMI_Init(HAL_DCMI_t* self);
void HAL_DCMI_SetCallback(HAL_DCMI_t* self, HAL_DCMI_CallbackIdx_t cb_idx, Callback_t* callback); 
HAL_DCMI_Status_t HAL_DCMI_StartDmaRecv(HAL_DCMI_t* self, uint8_t* buf, uint32_t len);
void HAL_DCMI_JpegCmd(HAL_DCMI_t* self, bool en);
void HAL_DCMI_CaptureCmd(HAL_DCMI_t* self, bool en);
bool HAL_DCMI_IsCapturing(HAL_DCMI_t* self);
void HAL_DCMI_Cmd(HAL_DCMI_t* self, bool en);
bool HAL_DCMI_IsEnabled(HAL_DCMI_t* self);
void HAL_DCMI_IRQHandler(HAL_DCMI_t* self);
void HAL_DCMI_Service(HAL_DCMI_t* self);

#endif
