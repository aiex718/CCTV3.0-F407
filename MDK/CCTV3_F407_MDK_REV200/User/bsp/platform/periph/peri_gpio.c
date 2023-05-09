#include "bsp/platform/periph/peri_gpio.h"

//GPIO LEDs
const HAL_GPIO_pin_t Peri_LED_STAT_pin_Inst = 
{	
	.GPIOx = GPIOC,
	.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
		.RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
	},
	.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
		.GPIO_Pin = GPIO_Pin_2,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_Speed = GPIO_Speed_2MHz,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL
	},
	.GPIO_AF_PinSource = 0,
	.GPIO_AF_Mapping = 0,
};//Peri_LED_STAT_pin_Inst
const HAL_GPIO_pin_t *Peri_LED_STAT_pin = &Peri_LED_STAT_pin_Inst;


