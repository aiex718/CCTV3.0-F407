#include "bsp/platform/periph/peri_timer_pwm.h"

const HAL_Timer_PWM_t Periph_Timer_PWM_FlashLight_Inst = 
{
	.Timer = __CONST_CAST_VAR(HAL_Timer_t)
	{
		.TIMx = TIM12,
		.Timer_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t)
		{
			.RCC_APB1Periph = RCC_APB1Periph_TIM12,
		},
		.Timer_InitCfg = __CONST_CAST_VAR(TIM_TimeBaseInitTypeDef)
		{
			.TIM_Prescaler = 42-1, //APB1 is 42MHz, divide 42 to get 1MHz
			.TIM_CounterMode = TIM_CounterMode_Up,
			.TIM_Period = (1000000/1000)-1, //pwm switch freq = 1MHz/1000 = 1KHz
			.TIM_ClockDivision = TIM_CKD_DIV1,
			.TIM_RepetitionCounter = 0,
		},
		.Timer_NVIC_InitCfg = NULL,//no interrupt enabled
		.Timer_Enable_ITs = __CONST_ARRAY_CAST_VAR(uint16_t){0}
	},
	.Timer_PWM_MaxChannelIdx = TIMER_PWM_CHANNEL_2,//TIM 12 has 2 channels
};

const HAL_Timer_PWM_t *Periph_Timer_PWM_FlashLight = &Periph_Timer_PWM_FlashLight_Inst;
