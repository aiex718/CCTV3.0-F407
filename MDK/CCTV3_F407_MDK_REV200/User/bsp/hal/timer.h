#ifndef TIMER_H
#define TIMER_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"

typedef struct 
{
    TIM_TypeDef* TIMx;
    HAL_RCC_Cmd_t *Timer_RCC_Cmd;
    TIM_TimeBaseInitTypeDef *Timer_InitCfg;
    NVIC_InitTypeDef *Timer_NVIC_InitCfg;
    uint16_t *Timer_Enable_ITs;
}HAL_Timer_t;

#define HAL_Timer_Cmd(timer,en) TIM_Cmd((timer)->TIMx,(en)?ENABLE:DISABLE)

void HAL_Timer_Init(HAL_Timer_t *self);

#endif
