#include "bsp/platform/device/dev_current_trig.h"

Device_CurrentTrig_t Dev_CurrentTrig_Inst = {
    .CurrentTrig_Timer = __CONST_CAST_VAR(HAL_Timer_t){
        .TIMx = TIM2,
        .Timer_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
            .RCC_APB1Periph = RCC_APB1Periph_TIM2,
        },
        .Timer_InitCfg = __CONST_CAST_VAR(TIM_TimeBaseInitTypeDef){
            .TIM_Prescaler = 84-1, //APB1 clock is 84MHz, div 84 to get 1MHz
            .TIM_CounterMode = TIM_CounterMode_Up,
            .TIM_Period = 10000-1,// 1Mhz/10000 = 100Hz
            .TIM_ClockDivision = TIM_CKD_DIV1,
            .TIM_RepetitionCounter = 0,
        },
        .Timer_NVIC_InitCfg = NULL,
        .Timer_Enable_ITs = NULL,
        .TIM_TRGOSource = TIM_TRGOSource_Update,
    },//CurrentTrig_Timer
    .CurrentTrig_ADC = __CONST_CAST_VAR(HAL_ADC_t){
        .ADCx = ADC2,
        .ADC_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
            .RCC_APB2Periph = RCC_APB2Periph_ADC2,
        },
        .ADC_GPIO_Pins = __CONST_ARRAY_CAST_VAR(HAL_GPIO_pin_t*)
        {
            __CONST_CAST_VAR(HAL_GPIO_pin_t)
            {
                .GPIOx = GPIOC,
                .GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
                    .RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
                },
                .GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
                    .GPIO_Pin = GPIO_Pin_3,
                    .GPIO_Mode = GPIO_Mode_AIN,
                    .GPIO_Speed = GPIO_Speed_2MHz,
                    .GPIO_OType = GPIO_OType_PP,
                    .GPIO_PuPd = GPIO_PuPd_NOPULL
                },
                .GPIO_AF_PinSource = 0,
                .GPIO_AF_Mapping = 0,
            },
            NULL, //NULL terminated
        },
        .ADC_InitCfg = __CONST_CAST_VAR(ADC_InitTypeDef){
            .ADC_Resolution = ADC_Resolution_8b,
            .ADC_ScanConvMode = DISABLE,
            .ADC_ContinuousConvMode = DISABLE,
            .ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising,
            .ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO,
            .ADC_DataAlign = ADC_DataAlign_Right,
            .ADC_NbrOfConversion = 1,
        },
        .ADC_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef){
            .NVIC_IRQChannel = ADC_IRQn,
            .NVIC_IRQChannelPreemptionPriority = 3,
            .NVIC_IRQChannelSubPriority = 2,
            .NVIC_IRQChannelCmd = ENABLE,
        },
        .ADC_Channel_list = __CONST_ARRAY_CAST_VAR(HAL_ADC_Channel_t*)
        {
            __CONST_CAST_VAR(HAL_ADC_Channel_t)
            {
                .ADC_Channel = ADC_Channel_13,
                .ADC_Channel_SampleTime = ADC_SampleTime_84Cycles,
                .ADC_Channel_Rank = 1,
            },
            NULL, //NULL terminated
        },
    },//CurrentTrig_ADC
    .CurrentTrig_DMA = __VAR_CAST_VAR(HAL_DMA_t)
    {
        .DMA_Streamx = DMA2_Stream3,
        .DMA_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t)
        {
            .RCC_AHB1Periph = RCC_AHB1Periph_DMA2,
        },
        .DMA_InitCfg = __CONST_CAST_VAR(DMA_InitTypeDef)
        {
            .DMA_Channel = DMA_Channel_1,
            .DMA_PeripheralBaseAddr = 0,
            .DMA_Memory0BaseAddr = 0,
            .DMA_DIR = DMA_DIR_PeripheralToMemory,
            .DMA_BufferSize = 0,
            .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
            .DMA_MemoryInc = DMA_MemoryInc_Enable,
            .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
            .DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
            .DMA_Mode = DMA_Mode_Circular,
            .DMA_Priority = DMA_Priority_Medium,
            .DMA_FIFOMode = DMA_FIFOMode_Enable,
            .DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull,
            .DMA_MemoryBurst = DMA_MemoryBurst_INC4,
            .DMA_PeripheralBurst = DMA_PeripheralBurst_Single,
        },
        .DMA_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef)
        {
            .NVIC_IRQChannel = DMA2_Stream3_IRQn,
            .NVIC_IRQChannelPreemptionPriority = 3,
            .NVIC_IRQChannelSubPriority = 2,
            .NVIC_IRQChannelCmd = ENABLE, 
        },
    },//CurrentTrig_DMA
    .CurrentTrig_Val_Buf = __VAR_ARRAY_CAST_VAR(float,500) {0},
    .CurrentTrig_Val_Buf_Len = 200,
    .CurrentTrig_ADC_Buf = __VAR_ARRAY_CAST_VAR(uint8_t,100) {0},
    .CurrentTrig_ADC_Buf_Len = 100,
    //Peak detection parameters
    .CurrentTrig_PeakThreshold = 1.5F,
    .CurrentTrig_PeakInfluence = 0.2F,
    //Threshold
    .CurrentTrig_Disconnect_Thres_mA = 2,
    .CurrentTrig_Overload_Thres_mA = 22
};

Device_CurrentTrig_t *Dev_CurrentTrig = &Dev_CurrentTrig_Inst;
