#ifndef RCC_H
#define RCC_H

#include "bsp/platform/platform_defs.h"

//BUS
__BSP_STRUCT_ALIGN typedef struct 
{
    uint32_t RCC_AHB1Periph;
    uint32_t RCC_AHB2Periph;
    uint32_t RCC_AHB3Periph;
    uint32_t RCC_APB1Periph;
    uint32_t RCC_APB2Periph;
}HAL_RCC_Cmd_t;

//MCO
typedef enum {
    MCO_NOT_SET = 0,
    MCO1,
    MCO2,
    __NOT_MCO_MAX,
}RCC_MCO_Idx_t;

__BSP_STRUCT_ALIGN typedef struct
{
    RCC_MCO_Idx_t MCO_Idx;
    struct HAL_GPIO_pin_struct *MCO_Pin;
    uint32_t MCO_Source;
    uint32_t MCO_ClkDiv;
}HAL_RCC_MCO_t;

//clk
typedef enum {
    CLK_NOT_SET = 0,
    CLK_HSE,
    CLK_HSI,
    CLK_LSE,
    CLK_LSI,
    __NOT_CLK_MAX,
}RCC_CLK_Idx_t;

__BSP_STRUCT_ALIGN typedef struct
{
    RCC_CLK_Idx_t CLK_Idx;
    bool CLK_Enable;
}HAL_RCC_CLK_t;

//main rcc setup struct
__BSP_STRUCT_ALIGN typedef struct 
{
    HAL_RCC_MCO_t **RCC_mco_list;
    HAL_RCC_CLK_t **RCC_clk_list;
}HAL_RCC_t;

void HAL_RCC_Cmd(const HAL_RCC_Cmd_t* cmd,const bool en);
void HAL_RCC_Init(HAL_RCC_t *self);

#endif
