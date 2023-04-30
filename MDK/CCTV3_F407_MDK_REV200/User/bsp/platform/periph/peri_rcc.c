#include "bsp/platform/periph/peri_rcc.h"
#include "bsp/hal/gpio.h"

const HAL_RCC_t Peri_RCC_Inst = 
{
	.RCC_mco_list = NULL,
	.RCC_clk_list = __CONST_ARRAY_CAST_VAR(HAL_RCC_CLK_t*)
	{
		//LSE for RTC
		__CONST_CAST_VAR(HAL_RCC_CLK_t){	
			.CLK_Idx = CLK_LSE,
			.CLK_Enable = true
		},
		NULL//Null terminate
	}
};

const HAL_RCC_t *Peri_RCC = &Peri_RCC_Inst;
