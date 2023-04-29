#ifndef RCC_H
#define RCC_H

#include "bsp/platform/platform_defs.h"

//BUS
__BSP_STRUCT_ALIGN typedef struct HAL_RCC_Cmd_s
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

__BSP_STRUCT_ALIGN typedef struct HAL_RCC_MCO_s
{
    RCC_MCO_Idx_t MCO_Idx;
    struct HAL_GPIO_pin_s *MCO_Pin;
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

__BSP_STRUCT_ALIGN typedef struct HAL_RCC_CLK_s
{
    RCC_CLK_Idx_t CLK_Idx;
    bool CLK_Enable;
}HAL_RCC_CLK_t;

//main rcc setup struct
__BSP_STRUCT_ALIGN typedef struct HAL_RCC_s
{
    HAL_RCC_MCO_t **RCC_mco_list;
    HAL_RCC_CLK_t **RCC_clk_list;
}HAL_RCC_t;

void HAL_RCC_Cmd(const HAL_RCC_Cmd_t* cmd,const bool en);
void HAL_RCC_Init(const HAL_RCC_t *self);

/* Example Usage

HAL_RCC_t *Periph_RCC = __CONST_CAST_VAR(HAL_RCC_t)
{
	.RCC_mco_list = __CONST_ARRAY_CAST_VAR(HAL_RCC_MCO_t*)
	{
		//MCO2
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
		NULL//Must Null terminate
	},
	.RCC_clk_list = __CONST_ARRAY_CAST_VAR(HAL_RCC_CLK_t*)
	{
		//LSE for RTC
		__CONST_CAST_VAR(HAL_RCC_CLK_t){	
			.CLK_Idx = CLK_LSE,
			.CLK_Enable = true
		},
		NULL//Must Null terminate
	}
};



*/

#endif
