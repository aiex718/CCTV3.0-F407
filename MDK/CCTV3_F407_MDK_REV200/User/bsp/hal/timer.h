#ifndef TIMER_H
#define TIMER_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"

typedef struct HAL_Timer_s
{
    TIM_TypeDef* TIMx;
    HAL_RCC_Cmd_t *Timer_RCC_Cmd;
    TIM_TimeBaseInitTypeDef *Timer_InitCfg;
    NVIC_InitTypeDef *Timer_NVIC_InitCfg;
    uint16_t *Timer_Enable_ITs;
    uint16_t TIM_TRGOSource;
}HAL_Timer_t;

__STATIC_INLINE bool HAL_Timer_IsEnabled(HAL_Timer_t *self)
{ 
    return (self->TIMx->CR1&TIM_CR1_CEN)?true:false;
}

void HAL_Timer_Init(HAL_Timer_t *self);
void HAL_Timer_Cmd(HAL_Timer_t *self,bool en);


#endif
