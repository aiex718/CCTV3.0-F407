#include "bsp/platform/device/dev_button.h"


Device_Button_t Dev_Button_Wkup_Inst = 
{
    .Button_GPIO_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
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
    },
    .Button_LongPress_Period = 3000,
    .Button_ShortPress_Period = 100,
    .Button_Scan_Period = 50,
    .Button_IdleState = false,//Idle low, active high
};

Device_Button_t *Dev_Button_Wkup = &Dev_Button_Wkup_Inst;
