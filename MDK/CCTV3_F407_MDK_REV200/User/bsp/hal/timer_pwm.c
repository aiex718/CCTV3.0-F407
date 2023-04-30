#include "bsp/hal/timer_pwm.h"

static const uint16_t Timer_PWM_ChannelIdx_to_EnBit_Map[__NOT_TIMER_PWM_CHANNEL_MAX] = {
    TIM_CCER_CC1E,
    TIM_CCER_CC2E,
    TIM_CCER_CC3E,
    TIM_CCER_CC4E,
};

void HAL_Timer_PWM_Init(const HAL_Timer_PWM_t *self)
{
    HAL_Timer_Init(self->Timer);
}

void HAL_Timer_PWM_InitChannel(const HAL_Timer_PWM_t *self,HAL_Timer_PWM_Channel_t *ch)
{
    if(ch->Timer_PWM_ChannelIdx <= self->Timer_PWM_MaxChannelIdx)
    {
        switch (ch->Timer_PWM_ChannelIdx)
        {
        case TIMER_PWM_CHANNEL_1:
            TIM_OC1Init(self->Timer->TIMx,ch->Timer_PWM_Channel_OCInitCfg);
            TIM_OC1PreloadConfig(self->Timer->TIMx,TIM_OCPreload_Enable);
            break;
        case TIMER_PWM_CHANNEL_2:
            TIM_OC2Init(self->Timer->TIMx,ch->Timer_PWM_Channel_OCInitCfg);
            TIM_OC2PreloadConfig(self->Timer->TIMx,TIM_OCPreload_Enable);
            break;
        case TIMER_PWM_CHANNEL_3:
            TIM_OC3Init(self->Timer->TIMx,ch->Timer_PWM_Channel_OCInitCfg);
            TIM_OC3PreloadConfig(self->Timer->TIMx,TIM_OCPreload_Enable);
            break;
        case TIMER_PWM_CHANNEL_4:
            TIM_OC4Init(self->Timer->TIMx,ch->Timer_PWM_Channel_OCInitCfg);
            TIM_OC4PreloadConfig(self->Timer->TIMx,TIM_OCPreload_Enable);
            break;
        default:
            break;
        }
    }
}

void HAL_Timer_PWM_ChannelCmd(const HAL_Timer_PWM_t *self,HAL_Timer_PWM_Channel_t *ch,bool en)
{
    if(ch->Timer_PWM_ChannelIdx <= self->Timer_PWM_MaxChannelIdx)
    {
        TIM_TypeDef *TIMx = (TIM_TypeDef *)self->Timer->TIMx;
        uint16_t en_bit = Timer_PWM_ChannelIdx_to_EnBit_Map[ch->Timer_PWM_ChannelIdx];
        if(en)
            TIMx->CCER |= en_bit;
        else
            TIMx->CCER &= ~en_bit;
    }
}

void HAL_Timer_PWM_SetDutyCycle(const HAL_Timer_PWM_t *self,
    HAL_Timer_PWM_Channel_t *ch,uint32_t duty_cycle)
{
    if(ch->Timer_PWM_ChannelIdx <= self->Timer_PWM_MaxChannelIdx)
    {
        TIM_TypeDef *TIMx = (TIM_TypeDef *)self->Timer->TIMx;
        switch (ch->Timer_PWM_ChannelIdx)
        {
        case TIMER_PWM_CHANNEL_1:
            TIMx->CCR1 = duty_cycle;
            break;
        case TIMER_PWM_CHANNEL_2:
            TIMx->CCR2 = duty_cycle;
            break;
        case TIMER_PWM_CHANNEL_3:
            TIMx->CCR3 = duty_cycle;
            break;
        case TIMER_PWM_CHANNEL_4:
            TIMx->CCR4 = duty_cycle;
            break;
        default:
            break;
        }
    }
}

uint32_t HAL_Timer_PWM_GetDutyCycle(const HAL_Timer_PWM_t *self,
    HAL_Timer_PWM_Channel_t *ch)
{
    if(ch->Timer_PWM_ChannelIdx <= self->Timer_PWM_MaxChannelIdx)
    {
        switch (ch->Timer_PWM_ChannelIdx)
        {
        case TIMER_PWM_CHANNEL_1:
            return self->Timer->TIMx->CCR1;
        case TIMER_PWM_CHANNEL_2:
            return self->Timer->TIMx->CCR2;
        case TIMER_PWM_CHANNEL_3:
            return self->Timer->TIMx->CCR3;
        case TIMER_PWM_CHANNEL_4:
            return self->Timer->TIMx->CCR4;
        default:
            break;
        }
    }   
    return 0;
}
uint32_t HAL_Timer_PWM_GetPeriod(const HAL_Timer_PWM_t *self)
{
    return self->Timer->TIMx->ARR+1;
}
