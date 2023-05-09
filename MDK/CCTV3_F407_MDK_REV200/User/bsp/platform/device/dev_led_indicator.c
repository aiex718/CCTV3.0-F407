#include "bsp/platform/device/dev_led_indicator.h"

Device_LedIndicator_t Dev_Led_Blink_Inst = 
{
    .LedIndicator_GPIO_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t)
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
    },
    .LedIndicator_Timer_Period = 1000,
};
Device_LedIndicator_t *Dev_Led_Blink = (Device_LedIndicator_t *)&Dev_Led_Blink_Inst;
