#include "bsp/platform/periph/peri_adc.h"

HAL_ADC_CommonCfg_t Peri_ADC_CommonCfg_Inst = {
	.ADC_CommonInitCfg = __CONST_CAST_VAR(ADC_CommonInitTypeDef){
		.ADC_Mode = ADC_Mode_Independent,
		.ADC_Prescaler = ADC_Prescaler_Div2,
		.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled,
		.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_10Cycles,
	},
	.ADC_EnableTempVrefint = true,
};

HAL_ADC_CommonCfg_t* Peri_ADC_CommonCfg = &Peri_ADC_CommonCfg_Inst;
