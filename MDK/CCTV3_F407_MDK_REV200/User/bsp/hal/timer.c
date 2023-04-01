#include "bsp/hal/timer.h"

//TODO:Test timer all
void HAL_Timer_Init(HAL_Timer_t *self)
{
    uint16_t *EnableITs = self->Timer_Enable_ITs;
    HAL_RCC_Cmd(self->Timer_RCC_Cmd,ENABLE);
    TIM_TimeBaseInit(self->TIMx,self->Timer_InitCfg);
    NVIC_Init(self->Timer_NVIC_InitCfg);

    while(*EnableITs)
        TIM_ITConfig(self->TIMx,*EnableITs++,ENABLE);
}

void HAL_Timer_Cmd(HAL_Timer_t *self,bool en)
{
    FunctionalState state = en?ENABLE:DISABLE;
    TIM_Cmd(self->TIMx,state);
    while((self->TIMx->CR1&TIM_CR1_CEN)!=state);
}
