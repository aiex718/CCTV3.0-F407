#ifndef DMA_H
#define DMA_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/array.h"
#include "bsp/hal/rcc.h"

#ifndef HAL_DMA_NUMOFDATA_LEN_T
    #define HAL_DMA_NUMOFDATA_LEN_T uint16_t
#endif

__BSP_STRUCT_ALIGN typedef struct
{
    DMA_Stream_TypeDef *DMA_Streamx;

    const HAL_RCC_Cmd_t *DMA_RCC_Cmd;
    const DMA_InitTypeDef *DMA_InitCfg;
    const NVIC_InitTypeDef *DMA_NVIC_InitCfg;
    uint16_t *DMA_Enable_ITs;
}HAL_DMA_t;

#define HAL_DMA_SetMemAddr(self,addr) \
    DMA_MemoryTargetConfig((self)->DMA_Streamx, (uint32_t)addr, DMA_Memory_0)
#define HAL_DMA_SetPeriphAddr(self,addr) do{\
    (self)->DMA_Streamx->PAR = (uint32_t)(addr); }while(0)
#define HAL_DMA_SetNumOfData(self, ndt) \
    DMA_SetCurrDataCounter((self)->DMA_Streamx,(ndt))
#define HAL_DMA_GetNumOfData(self) \
    DMA_GetCurrDataCounter((self)->DMA_Streamx)
#define HAL_DMA_Cmd(self,en) do{                                \
    if(en) DMA_Cmd((self)->DMA_Streamx, ENABLE);                \
    else{ DMA_Cmd((self)->DMA_Streamx, DISABLE);                \
    while(DMA_GetCmdStatus((self)->DMA_Streamx) != DISABLE);}   \
}while(0)
void HAL_DMA_Init(const HAL_DMA_t *self);

#endif
