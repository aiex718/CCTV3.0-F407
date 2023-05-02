#include "bsp/platform/periph/peri_rng.h"

#include "bsp/hal/rcc.h"

const HAL_Rng_t Peri_Rng_Inst = {
	.Rng_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t)
	{
		.RCC_AHB2Periph = RCC_AHB2Periph_RNG,
	},
};

const HAL_Rng_t *Peri_Rng = &Peri_Rng_Inst;
