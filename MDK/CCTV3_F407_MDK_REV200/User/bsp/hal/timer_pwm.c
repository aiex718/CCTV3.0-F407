#include "bsp/hal/timer_pwm.h"

//TODO:Test timer pwm all

void HAL_Timer_PWM_Init(HAL_Timer_PWM_t *self)
{
    HAL_Timer_Init(self->Timer);

    for (HAL_Timer_PWM_Channel_t ch = (HAL_Timer_PWM_Channel_t)0 ;
            ch < __NOT_PWM_CHANNEL_MAX; ch++)
    {
        if(self->Timer_PWM_pins[ch])
        {
            HAL_GPIO_InitPin(self->Timer_PWM_pins[ch]);
            switch (ch)
            {
            case TIMER_PWM_CHANNEL_1:
                TIM_OC1Init(self->Timer->TIMx,self->Timer_PWM_InitCfg);
                TIM_OC1PreloadConfig(self->Timer->TIMx,TIM_OCPreload_Enable);
                break;
            case TIMER_PWM_CHANNEL_2:
                TIM_OC2Init(self->Timer->TIMx,self->Timer_PWM_InitCfg);
                TIM_OC2PreloadConfig(self->Timer->TIMx,TIM_OCPreload_Enable);
                break;
            case TIMER_PWM_CHANNEL_3:
                TIM_OC3Init(self->Timer->TIMx,self->Timer_PWM_InitCfg);
                TIM_OC3PreloadConfig(self->Timer->TIMx,TIM_OCPreload_Enable);
                break;
            case TIMER_PWM_CHANNEL_4:
                TIM_OC4Init(self->Timer->TIMx,self->Timer_PWM_InitCfg);
                TIM_OC4PreloadConfig(self->Timer->TIMx,TIM_OCPreload_Enable);
                break;
            default:
                break;
            }
        }
    }
}

void HAL_Timer_PWM_SetDutyCycle(HAL_Timer_PWM_t *self,
    HAL_Timer_PWM_Channel_t ch,uint32_t duty_cycle)
{
    switch (ch)
    {
    case TIMER_PWM_CHANNEL_1:
        self->Timer->TIMx->CCR1 = duty_cycle;
        break;
    case TIMER_PWM_CHANNEL_2:
        self->Timer->TIMx->CCR2 = duty_cycle;
        break;
    case TIMER_PWM_CHANNEL_3:
        self->Timer->TIMx->CCR3 = duty_cycle;
        break;
    case TIMER_PWM_CHANNEL_4:
        self->Timer->TIMx->CCR4 = duty_cycle;
        break;
    default:
        break;
    }
}

uint32_t HAL_Timer_PWM_GetDutyCycle(HAL_Timer_PWM_t *self,HAL_Timer_PWM_Channel_t ch)
{
    switch (ch)
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
    return 0;
}
