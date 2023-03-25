#ifndef GPIO_H
#define GPIO_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"

__BSP_STRUCT_ALIGN typedef struct
{
    //RCC
    HAL_RCC_Cmd_t* GPIO_RCC_cmd;
    //GPIO
    GPIO_TypeDef* GPIOx;
    GPIO_InitTypeDef* GPIO_InitCfg;
    //AF
    uint16_t GPIO_AF_PinSource;
    uint8_t GPIO_AF_Mapping;
}HAL_GPIO_pin_t;

void HAL_GPIO_InitPin(const HAL_GPIO_pin_t* gpio);
#define HAL_GPIO_WritePin(gpio,val) GPIO_WriteBit((gpio)->GPIOx,(gpio)->GPIO_InitCfg->GPIO_Pin,(BitAction)(val))
#define HAL_GPIO_ReadPin(gpio) GPIO_ReadInputDataBit((gpio)->GPIOx,(gpio)->GPIO_InitCfg->GPIO_Pin)
#define HAL_GPIO_TogglePin(gpio) GPIO_ToggleBits((gpio)->GPIOx,(gpio)->GPIO_InitCfg->GPIO_Pin)

#endif
