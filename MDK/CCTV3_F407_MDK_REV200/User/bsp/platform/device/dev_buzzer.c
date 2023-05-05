#include "bsp/platform/device/dev_buzzer.h"

Device_Buzzer_t Dev_Buzzer_Inst={
    .Buzzer_GPIO_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t)
	{
		.GPIOx = GPIOA,
		.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_AHB1Periph = RCC_AHB1Periph_GPIOA,
		},
		.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
			.GPIO_Pin = GPIO_Pin_3,
			.GPIO_Mode = GPIO_Mode_OUT,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
	},
    .Buzzer_Buffer = __VAR_CAST_VAR(Buffer_uint16_t)
    {
        .buf_ptr = __VAR_ARRAY_CAST_VAR(uint16_t,10){
            0
        },
        .len=10
    } 
};

Device_Buzzer_t *Dev_Buzzer = &Dev_Buzzer_Inst;

