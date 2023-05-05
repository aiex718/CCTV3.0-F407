#ifndef ADC_H
#define ADC_H

#include "bsp/platform/platform_defs.h"

#include "bsp/hal/rcc.h"
#include "bsp/hal/dma.h"
#include "bsp/hal/gpio.h"
#include "bsp/sys/callback.h"

typedef struct HAL_ADC_CommonCfg_s
{
    ADC_CommonInitTypeDef *ADC_CommonInitCfg;
    bool ADC_EnableTempVrefint;
}HAL_ADC_CommonCfg_t;

void HAL_ADC_CommonInit(HAL_ADC_CommonCfg_t* self);

typedef enum 
{
    //ADC callbacks always invoked in ISR
    //param is uint16_t ADC value ptr
    HAL_ADC_CALLBACK_IRQ_EOC    = 0 ,
    HAL_ADC_CALLBACK_IRQ_JEOC       ,
    HAL_ADC_CALLBACK_IRQ_AWD        ,
    HAL_ADC_CALLBACK_IRQ_OVR        ,
    
    __NOT_CALLBACK_ADC_MAX  ,
}HAL_ADC_CallbackIdx_t;

typedef struct HAL_ADC_Channel_s
{
    uint8_t ADC_Channel;
    uint8_t ADC_Channel_Rank;
    uint8_t ADC_Channel_SampleTime;
}HAL_ADC_Channel_t;

typedef struct HAL_ADC_s
{
    ADC_TypeDef* ADCx;
    HAL_RCC_Cmd_t *ADC_RCC_Cmd;
    ADC_InitTypeDef *ADC_InitCfg;
    HAL_GPIO_pin_t **ADC_GPIO_Pins;
    NVIC_InitTypeDef *ADC_NVIC_InitCfg;
    HAL_ADC_Channel_t **ADC_Channel_list;
    Callback_t *ADC_Callbacks[__NOT_CALLBACK_ADC_MAX];
}HAL_ADC_t;

__STATIC_INLINE void HAL_ADC_Cmd(HAL_ADC_t* self,bool en)
{
    ADC_Cmd(self->ADCx,en?ENABLE:DISABLE);
}

__STATIC_INLINE void HAL_ADC_DMACmd(HAL_ADC_t* self,bool en)
{
    ADC_DMACmd(self->ADCx,en?ENABLE:DISABLE);
    ADC_DMARequestAfterLastTransferCmd(self->ADCx,en?ENABLE:DISABLE);
}

__STATIC_INLINE void HAL_ADC_SoftwareTrigCmd(HAL_ADC_t* self)
{
    ADC_SoftwareStartConv(self->ADCx);
}

__STATIC_INLINE void HAL_ADC_ContinuousModeCmd(HAL_ADC_t* self,bool en)
{
    ADC_ContinuousModeCmd(self->ADCx,en?ENABLE:DISABLE);
}

__STATIC_INLINE void* HAL_ADC_GetDmaAddr(HAL_ADC_t* self)
{
    return (void*)&(self->ADCx->DR);
}

__STATIC_INLINE uint16_t HAL_ADC_GetConvertValue(HAL_ADC_t* self)
{
    return ADC_GetConversionValue(self->ADCx);
}

void HAL_ADC_Init(HAL_ADC_t* self);
void HAL_ADC_SetCallback(HAL_ADC_t* self,HAL_ADC_CallbackIdx_t cb_idx,Callback_t *cb);
void HAL_ADC_IRQHandler(HAL_ADC_t* self);

uint16_t HAL_ADC_InjectConvertValue_Polling
    (HAL_ADC_t* self,uint8_t ch,uint8_t sample_time);


#endif
