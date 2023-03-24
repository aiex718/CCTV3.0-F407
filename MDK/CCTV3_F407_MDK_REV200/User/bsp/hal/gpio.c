#include "bsp/hal/gpio.h"


void HAL_GPIO_InitPin(const HAL_GPIO_pin_t* gpio)
{
    HAL_RCC_Cmd(gpio->GPIO_RCC_cmd,true);
    GPIO_Init(gpio->GPIOx, gpio->GPIO_InitCfg);
    if(gpio->GPIO_InitCfg->GPIO_Mode == GPIO_Mode_AF)
        GPIO_PinAFConfig(gpio->GPIOx,gpio->GPIO_AF_PinSource,gpio->GPIO_AF_Mapping);
}
