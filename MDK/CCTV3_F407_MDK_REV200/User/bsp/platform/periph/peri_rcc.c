#include "bsp/platform/periph/peri_rcc.h"
#include "bsp/hal/gpio.h"

const HAL_RCC_t Peri_RCC_Inst = 
{
	.RCC_MCO_list = __CONST_ARRAY_CAST_VAR(HAL_RCC_MCO_t*)
	{
		//MCO2 for camera
		__CONST_CAST_VAR(HAL_RCC_MCO_t){
			.MCO_Idx = MCO2, 
			.MCO_Pin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOC, //MCO2 - PC9
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){
					.GPIO_Pin = GPIO_Pin_9,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_100MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource9,
				.GPIO_AF_Mapping = GPIO_AF_MCO,
			},
			.MCO_Source = RCC_MCO2Source_HSE,
			.MCO_ClkDiv = RCC_MCO2Div_1,
		},
		NULL//Null terminate
	},
	.RCC_CLK_list = NULL
};

const HAL_RCC_t *Peri_RCC = &Peri_RCC_Inst;
