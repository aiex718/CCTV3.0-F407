#ifndef DMA_H
#define DMA_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/callback.h"
#include "bsp/hal/rcc.h"

#ifndef HAL_DMA_NUMOFDATA_LEN_T
    #define HAL_DMA_NUMOFDATA_LEN_T uint16_t
#endif

typedef enum 
{
    //IRQ callbacks always invoked in ISR
    DMA_CALLBACK_IRQ_HT = 0 ,//Half Transfer
    DMA_CALLBACK_IRQ_TC     ,//Transfer Complete
    DMA_CALLBACK_IRQ_TE     ,//Transfer Error
    DMA_CALLBACK_IRQ_FE     ,//FIFO Error
    DMA_CALLBACK_IRQ_DME    ,//Direct Mode Error
    __NOT_CALLBACK_DMA_MAX
}HAL_DMA_CallbackIdx_t;

__BSP_STRUCT_ALIGN typedef struct
{
    DMA_Stream_TypeDef *DMA_Streamx;

    HAL_RCC_Cmd_t *DMA_RCC_Cmd;
    DMA_InitTypeDef *DMA_InitCfg;
    NVIC_InitTypeDef *DMA_NVIC_InitCfg;

    Callback_t* DMA_Callbacks[__NOT_CALLBACK_DMA_MAX];
    //customize data structure
    void* pExtension;

    //Private data, do not modify
    uint32_t* __it_status_flags;
}HAL_DMA_t;

#define HAL_DMA_SetMemAddr(self,addr) \
    DMA_MemoryTargetConfig((self)->DMA_Streamx, (uint32_t)addr, DMA_Memory_0)
#define HAL_DMA_SetPeriphAddr(self,addr) do{\
    (self)->DMA_Streamx->PAR = (uint32_t)(addr); }while(0)
#define HAL_DMA_SetNumOfData(self, ndt) \
    DMA_SetCurrDataCounter((self)->DMA_Streamx,(ndt))
#define HAL_DMA_GetNumOfData(self) \
    DMA_GetCurrDataCounter((self)->DMA_Streamx)
#define HAL_DMA_ClearCallback(dma,cb) HAL_DMA_SetCallback((dma),(cb),NULL)

void HAL_DMA_Init(HAL_DMA_t *self);
void HAL_DMA_DeInit(const HAL_DMA_t *self);
void HAL_DMA_ReloadCfg(const HAL_DMA_t *self);
void HAL_DMA_Cmd(const HAL_DMA_t *self, bool en);
void HAL_DMA_SetCallback(HAL_DMA_t* self, HAL_DMA_CallbackIdx_t cb_idx, Callback_t* callback); 
void HAL_DMA_IRQHandler(HAL_DMA_t* self);
void HAL_DMA_Service(const HAL_DMA_t *dma);


#endif
