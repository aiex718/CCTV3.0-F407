#ifndef TIMER_PWM_H
#define TIMER_PWM_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/rcc.h"
#include "bsp/hal/timer.h"

typedef enum 
{
    TIMER_PWM_CHANNEL_1=0,
    TIMER_PWM_CHANNEL_2,
    TIMER_PWM_CHANNEL_3,
    TIMER_PWM_CHANNEL_4,
    __NOT_TIMER_PWM_CHANNEL_MAX,
}HAL_Timer_PWM_ChannelIdx_t;

typedef struct HAL_Timer_PWM_s
{
    HAL_Timer_t *Timer;
    HAL_Timer_PWM_ChannelIdx_t Timer_PWM_MaxChannelIdx;
}HAL_Timer_PWM_t;

typedef struct HAL_Timer_PWM_Channel_s
{
    HAL_Timer_PWM_ChannelIdx_t Timer_PWM_ChannelIdx;
    TIM_OCInitTypeDef *Timer_PWM_Channel_OCInitCfg;
}HAL_Timer_PWM_Channel_t;

#define HAL_Timer_PWM_Cmd(self,en) HAL_Timer_Cmd((self)->Timer,(en))

void HAL_Timer_PWM_Init(const HAL_Timer_PWM_t *self);
void HAL_Timer_PWM_InitChannel(const HAL_Timer_PWM_t *self,HAL_Timer_PWM_Channel_t *ch);
void HAL_Timer_PWM_ChannelCmd(const HAL_Timer_PWM_t *self,HAL_Timer_PWM_Channel_t *ch,bool en);
void HAL_Timer_PWM_SetDutyCycle(const HAL_Timer_PWM_t *self,HAL_Timer_PWM_Channel_t *ch,uint32_t duty_cycle);
uint32_t HAL_Timer_PWM_GetDutyCycle(const HAL_Timer_PWM_t *self,HAL_Timer_PWM_Channel_t *ch);
uint32_t HAL_Timer_PWM_GetPeriod(const HAL_Timer_PWM_t *self);



#endif
