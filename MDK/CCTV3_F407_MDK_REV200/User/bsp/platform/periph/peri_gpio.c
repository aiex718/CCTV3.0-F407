#include "bsp/platform/periph/peri_gpio.h"

//GPIO LEDs
const HAL_GPIO_pin_t _Periph_LED_Load_pin = 
{
	.GPIOx = GPIOC,
	.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
		.RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
	},
	.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
		.GPIO_Pin = GPIO_Pin_0,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_Speed = GPIO_Speed_2MHz,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL
	},
	.GPIO_AF_PinSource = 0,
	.GPIO_AF_Mapping = 0,
};//_Periph_LED_Load_pin
const HAL_GPIO_pin_t *Periph_LED_Load_pin = &_Periph_LED_Load_pin;

const HAL_GPIO_pin_t _Periph_LED_STAT_pin = 
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
};//_Periph_LED_STAT_pin
const HAL_GPIO_pin_t *Periph_LED_STAT_pin = &_Periph_LED_STAT_pin;

//GPIO Buttons
const HAL_GPIO_pin_t _Periph_Button_Wkup_pin = 
{
	.GPIOx = GPIOA,
	.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
		.RCC_AHB1Periph = RCC_AHB1Periph_GPIOA,
	},
	.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
		.GPIO_Pin = GPIO_Pin_0,
		.GPIO_Mode = GPIO_Mode_IN,
		.GPIO_Speed = GPIO_Speed_2MHz,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL
	},
	.GPIO_AF_PinSource = 0,
	.GPIO_AF_Mapping = 0,
};
const HAL_GPIO_pin_t *Periph_Button_Wkup_pin = &_Periph_Button_Wkup_pin;
