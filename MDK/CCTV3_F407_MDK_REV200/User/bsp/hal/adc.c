#include "bsp/hal/adc.h"

static const uint16_t ADC_CBidx_to_ITFlag_Tbl[__NOT_CALLBACK_ADC_MAX] = {
    ADC_IT_EOC,
    ADC_IT_JEOC,
    ADC_IT_AWD,
    ADC_IT_OVR,
};

void HAL_ADC_CommonInit(HAL_ADC_CommonCfg_t* self)
{
    ADC_CommonInit(self->ADC_CommonInitCfg);
    ADC_TempSensorVrefintCmd(self->ADC_EnableTempVrefint?ENABLE:DISABLE);
}

void HAL_ADC_Init(HAL_ADC_t* self)
{
    HAL_RCC_Cmd(self->ADC_RCC_Cmd,ENABLE);
    BSP_MEMSET(self->ADC_Callbacks,0,sizeof(self->ADC_Callbacks));

    ADC_Init(self->ADCx,self->ADC_InitCfg);

    if(self->ADC_GPIO_Pins)
    {
        HAL_GPIO_pin_t **gpio_pins = self->ADC_GPIO_Pins;
        while(*gpio_pins)
            HAL_GPIO_InitPin(*gpio_pins++);
    }    

    if(self->ADC_NVIC_InitCfg)
        NVIC_Init(self->ADC_NVIC_InitCfg);

    if(self->ADC_Channel_list)
    {
        HAL_ADC_Channel_t **adc_channel_list = self->ADC_Channel_list;
        HAL_ADC_Channel_t *adc_ch;
        while(*adc_channel_list)
        {
            adc_ch = *adc_channel_list++;
            ADC_RegularChannelConfig(self->ADCx, adc_ch->ADC_Channel, adc_ch->ADC_Channel_Rank, adc_ch->ADC_Channel_SampleTime);
        }
    }

}

void HAL_ADC_SetCallback(HAL_ADC_t* self,HAL_ADC_CallbackIdx_t cb_idx,Callback_t *cb)
{
    if(cb_idx < __NOT_CALLBACK_ADC_MAX)
    {
        uint16_t itFlag = ADC_CBidx_to_ITFlag_Tbl[cb_idx];

        ADC_ClearITPendingBit(self->ADCx,itFlag);
        ADC_ITConfig(self->ADCx,itFlag,cb?ENABLE:DISABLE);
        self->ADC_Callbacks[cb_idx] = cb;
    }
}

uint16_t HAL_ADC_InjectConvertValue_Polling
    (HAL_ADC_t* self,uint8_t ch,uint8_t sample_time)
{
    ADC_InjectedChannelConfig(self->ADCx, ch, 1, sample_time);
	ADC_SoftwareStartInjectedConv(self->ADCx);
	while(!ADC_GetFlagStatus(self->ADCx, ADC_FLAG_JEOC));
	return ADC_GetInjectedConversionValue(self->ADCx, ADC_InjectedChannel_1);
}

void HAL_ADC_IRQHandler(HAL_ADC_t* self)
{
    uint16_t itFlag;
    uint8_t idx;

    for (idx = 0; idx < BSP_ARR_LEN(ADC_CBidx_to_ITFlag_Tbl); idx++)
    {
        itFlag = ADC_CBidx_to_ITFlag_Tbl[idx];

        if(ADC_GetITStatus(self->ADCx,itFlag))
        {
            ADC_ClearITPendingBit(self->ADCx,itFlag);

            if(self->ADC_Callbacks[idx])
            {
                uint16_t adc_val = ADC_GetConversionValue(self->ADCx);
                Callback_Invoke_Idx(self,&adc_val,self->ADC_Callbacks,idx);
            }
        }
    }
}
