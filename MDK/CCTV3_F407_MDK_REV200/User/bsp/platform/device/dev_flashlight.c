#include "bsp/platform/device/dev_flashlight.h"

Device_FlashLight_t Dev_FlashLight_Top_Inst = 
{
	.FlashLight_Timer_PWM = NULL,
	.FlashLight_Timer_PWM_Channel = __CONST_CAST_VAR(HAL_Timer_PWM_Channel_t){
		.Timer_PWM_ChannelIdx = TIMER_PWM_CHANNEL_1,//CH1
		.Timer_PWM_Channel_OCInitCfg = __CONST_CAST_VAR(TIM_OCInitTypeDef)
		{
			.TIM_OCMode = TIM_OCMode_PWM1,
			.TIM_OutputState = TIM_OutputState_Enable,
			.TIM_OutputNState = TIM_OutputNState_Disable,
			.TIM_Pulse = 0, //default duty cycle to 0
			.TIM_OCPolarity = TIM_OCPolarity_High,//CNT > CCR, output will set high 
			//.TIM_OCNPolarity = TIM_OCNPolarity_High, 
			//.TIM_OCIdleState = TIM_OCIdleState_Reset,
			//.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
		}
	},
	.FlashLight_GPIO_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t) //PB14
	{
		.GPIOx = GPIOB,
		.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_AHB1Periph = RCC_AHB1Periph_GPIOB,
		},
		.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
			.GPIO_Pin = GPIO_Pin_14,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
		.GPIO_AF_PinSource = GPIO_PinSource14,
		.GPIO_AF_Mapping = GPIO_AF_TIM12,
	},
	.FlashLight_Brightness = 1,//default brightness 1%
};
Device_FlashLight_t *Dev_FlashLight_Top = &Dev_FlashLight_Top_Inst;


Device_FlashLight_t Dev_FlashLight_Bottom_Inst = 
{
	.FlashLight_Timer_PWM = NULL,
	.FlashLight_Timer_PWM_Channel = __CONST_CAST_VAR(HAL_Timer_PWM_Channel_t){
		.Timer_PWM_ChannelIdx = TIMER_PWM_CHANNEL_2,//CH2
		.Timer_PWM_Channel_OCInitCfg = __CONST_CAST_VAR(TIM_OCInitTypeDef)
		{
			.TIM_OCMode = TIM_OCMode_PWM1,
			.TIM_OutputState = TIM_OutputState_Enable,
			.TIM_OutputNState = TIM_OutputNState_Disable,
			.TIM_Pulse = 0, //default duty cycle to 0
			.TIM_OCPolarity = TIM_OCPolarity_High,//CNT > CCR, output will set high 
			//.TIM_OCNPolarity = TIM_OCNPolarity_High, TODO: check this value
			//.TIM_OCIdleState = TIM_OCIdleState_Reset,
			//.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
		}
	},
	.FlashLight_GPIO_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t) //PB15
	{
		.GPIOx = GPIOB,
		.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_AHB1Periph = RCC_AHB1Periph_GPIOB,
		},
		.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
			.GPIO_Pin = GPIO_Pin_15,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
		.GPIO_AF_PinSource = GPIO_PinSource15,
		.GPIO_AF_Mapping = GPIO_AF_TIM12,
	},
	.FlashLight_Brightness = 1,//default brightness 1%
};

Device_FlashLight_t *Dev_FlashLight_Bottom = &Dev_FlashLight_Bottom_Inst;
