#include "bsp/platform/periph_list.h"

#include "bsp/sys/concurrent_queue.h"
#include "bsp/sys/callback.h"

#include "bsp/hal/rcc.h"

#define __VAR_CAST_VAR(type) (type*)&(type)
#define __VAR_ARRAY_CAST_VAR(type,len) (type*)&(type[len])
#define __CONST_CAST_VAR(type) (type*)&(const type)
#define __CONST_ARRAY_CAST_VAR(type) (type*)&(const type[])

//GPIO LEDs
HAL_GPIO_pin_t *Periph_LED_Load_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t)
{
	.GPIOx = GPIOC,
	.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
		.RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
	},
	.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
		.GPIO_Pin = GPIO_Pin_0,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_Speed = GPIO_Speed_2MHz,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL
	},
	.GPIO_AF_PinSource = 0,
	.GPIO_AF_Mapping = 0,
};

HAL_GPIO_pin_t *Periph_LED_STAT_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t)
{	
	.GPIOx = GPIOC,
	.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
		.RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
	},
	.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
		.GPIO_Pin = GPIO_Pin_2,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_Speed = GPIO_Speed_2MHz,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL
	},
	.GPIO_AF_PinSource = 0,
	.GPIO_AF_Mapping = 0,
};

//GPIO Buttons
HAL_GPIO_pin_t *Periph_Button_Wkup_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t)
{
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
};

DBG_Serial_t* DBG_Serial= __VAR_CAST_VAR(DBG_Serial_t)
{
	.tx_con_queue = __VAR_CAST_VAR(Concurrent_Queue_uint8_t)
	{
		.mem = __VAR_ARRAY_CAST_VAR(uint8_t,DEBUG_SERIAL_TX_BUFFER_SIZE)
		{
			0
		},
		.mem_len = DEBUG_SERIAL_TX_BUFFER_SIZE,
		.allow_push_preempt = 1,
	},
	.rx_con_queue = __VAR_CAST_VAR(Concurrent_Queue_uint8_t)
	{
		.mem = __VAR_ARRAY_CAST_VAR(uint8_t,DEBUG_SERIAL_RX_BUFFER_SIZE)
		{
			0
		},
		.mem_len = DEBUG_SERIAL_RX_BUFFER_SIZE,
		.allow_push_preempt = 0,
	},
	.hal_usart = __VAR_CAST_VAR(HAL_USART_t)
	{
		.USARTx = USART3,
		.USART_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_APB1Periph = RCC_APB1Periph_USART3,
		},
		.USART_TxPin = __CONST_CAST_VAR(HAL_GPIO_pin_t)
		{
			.GPIOx = GPIOD,
			.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
				.RCC_AHB1Periph = RCC_AHB1Periph_GPIOD,
			},
			.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
				.GPIO_Pin = GPIO_Pin_8,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
			.GPIO_AF_PinSource = GPIO_PinSource8,
			.GPIO_AF_Mapping = GPIO_AF_USART3,
		},
		.USART_RxPin = __CONST_CAST_VAR(HAL_GPIO_pin_t)
		{
			.GPIOx = GPIOD,
			.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
				.RCC_AHB1Periph = RCC_AHB1Periph_GPIOD,
			},
			.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
				.GPIO_Pin = GPIO_Pin_9,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
			.GPIO_AF_PinSource = GPIO_PinSource9,
			.GPIO_AF_Mapping = GPIO_AF_USART3,
		},
		.USART_InitCfg = __CONST_CAST_VAR(USART_InitTypeDef)
		{
			.USART_BaudRate = 115200,
			.USART_WordLength = USART_WordLength_8b,
			.USART_StopBits = USART_StopBits_1,
			.USART_Parity = USART_Parity_No,
			.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
		},
		.USART_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef)
		{
			.NVIC_IRQChannel = USART3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 0,
			.NVIC_IRQChannelSubPriority = 2,
			.NVIC_IRQChannelCmd = ENABLE,
		},
		.USART_Enable_ITs = __CONST_ARRAY_CAST_VAR(uint16_t)
		{
			0
		},
		.USART_TxDma_Cfg = __CONST_CAST_VAR(HAL_DMA_t)
		{
			.DMA_Streamx = DMA1_Stream3,
			.DMA_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t)
			{
				.RCC_AHB1Periph = RCC_AHB1Periph_DMA1,
			},
			.DMA_InitCfg = __CONST_CAST_VAR(DMA_InitTypeDef)
			{
				.DMA_Channel = DMA_Channel_4,
				.DMA_PeripheralBaseAddr = 0,
				.DMA_Memory0BaseAddr = 0,
				.DMA_DIR = DMA_DIR_MemoryToPeripheral,
				.DMA_BufferSize = 0,
				.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
				.DMA_Mode = DMA_Mode_Normal,
				.DMA_Priority = DMA_Priority_Low,
				.DMA_FIFOMode = DMA_FIFOMode_Enable,
				.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull,
				.DMA_MemoryBurst = DMA_MemoryBurst_INC8,
				.DMA_PeripheralBurst = DMA_PeripheralBurst_Single,
			},
			//TODO: set unused DMA_NVIC_InitCfg to null
			.DMA_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef)
			{
				.NVIC_IRQChannel = DMA1_Stream3_IRQn,
				.NVIC_IRQChannelPreemptionPriority = 2,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = DISABLE,//managed by USART 
			},
		},
		.USART_RxDma_Cfg = __CONST_CAST_VAR(HAL_DMA_t)
		{
			.DMA_Streamx = DMA1_Stream1,
			.DMA_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t)
			{
				.RCC_AHB1Periph = RCC_AHB1Periph_DMA1,
			},
			.DMA_InitCfg = __CONST_CAST_VAR(DMA_InitTypeDef)
			{
				.DMA_Channel = DMA_Channel_4,
				.DMA_PeripheralBaseAddr = 0,
				.DMA_Memory0BaseAddr = 0,
				.DMA_DIR = DMA_DIR_PeripheralToMemory,
				.DMA_BufferSize = 0,
				.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
				.DMA_Mode = DMA_Mode_Normal,
				.DMA_Priority = DMA_Priority_Low,
				.DMA_FIFOMode = DMA_FIFOMode_Enable,
				.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull,
				.DMA_MemoryBurst = DMA_MemoryBurst_INC8,
				.DMA_PeripheralBurst = DMA_PeripheralBurst_Single,
			},
			//TODO: set unused DMA_NVIC_InitCfg to null
			.DMA_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef)
			{
				.NVIC_IRQChannel = DMA1_Stream1_IRQn,
				.NVIC_IRQChannelPreemptionPriority = 2,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = DISABLE, 
			},
		},
		.USART_Tx_Buf = __VAR_CAST_VAR(Buffer_uint8_t)
		{
			.buf_ptr = __VAR_ARRAY_CAST_VAR(uint8_t,DEBUG_USART3_TX_BUFFER_SIZE)
			{
				0
			},
			.len=DEBUG_USART3_TX_BUFFER_SIZE
		},
		.USART_Rx_Buf = __VAR_CAST_VAR(Buffer_uint8_t)
		{
			.buf_ptr = __VAR_ARRAY_CAST_VAR(uint8_t,DEBUG_USART3_RX_BUFFER_SIZE)
			{
				0
			},
			.len=DEBUG_USART3_RX_BUFFER_SIZE
		},
		.USART_Rx_Threshold = 0,
		.USART_Rx_Timeout = 100,
		.pExtension = NULL,
	},//hal_usart
};

//Timer PWM
HAL_Timer_PWM_t *Periph_Timer_PWM_FlashLight = __CONST_CAST_VAR(HAL_Timer_PWM_t)
{
	.Timer = __CONST_CAST_VAR(HAL_Timer_t)
	{
		.TIMx = TIM12,
		.Timer_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t)
		{
			.RCC_APB1Periph = RCC_APB1Periph_TIM12,
		},
		.Timer_InitCfg = __CONST_CAST_VAR(TIM_TimeBaseInitTypeDef)
		{
			.TIM_Prescaler = 42-1, //APB1 is 42MHz, divide 42 to get 1MHz
			.TIM_CounterMode = TIM_CounterMode_Up,
			.TIM_Period = (1000000/1000)-1, //pwm switch freq = 1MHz/1000 = 1KHz
			.TIM_ClockDivision = TIM_CKD_DIV1,
			.TIM_RepetitionCounter = 0,
		},
		.Timer_NVIC_InitCfg = NULL,//no interrupt enabled
		.Timer_Enable_ITs = __CONST_ARRAY_CAST_VAR(uint16_t){0}
	},
	.Timer_PWM_MaxChannelIdx = TIMER_PWM_CHANNEL_2,//TIM 12 has 2 channels
};

//Device flashlight
Device_FlashLight_t *Periph_FlashLight_Top = __VAR_CAST_VAR(Device_FlashLight_t)
{
	.FlashLight_Timer_PWM = NULL,
	.FlashLight_Timer_PWM_Channel = __CONST_CAST_VAR(HAL_Timer_PWM_Channel_t){
		.Timer_PWM_ChannelIdx = TIMER_PWM_CHANNEL_1,//CH1
		.Timer_PWM_Channel_OCInitCfg = __CONST_CAST_VAR(TIM_OCInitTypeDef)
		{
			.TIM_OCMode = TIM_OCMode_PWM1,
			.TIM_OutputState = TIM_OutputState_Enable,
			.TIM_OutputNState = TIM_OutputNState_Disable,
			.TIM_Pulse = 0, //default duty cycle to 0
			.TIM_OCPolarity = TIM_OCPolarity_High,//CNT > CCR, output will set high 
			//.TIM_OCNPolarity = TIM_OCNPolarity_High, TODO: check this value
			//.TIM_OCIdleState = TIM_OCIdleState_Reset,
			//.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
		}
	},
	.FlashLight_GPIO_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t) //PB14
	{
		.GPIOx = GPIOB,
		.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_AHB1Periph = RCC_AHB1Periph_GPIOB,
		},
		.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
			.GPIO_Pin = GPIO_Pin_14,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
		.GPIO_AF_PinSource = GPIO_PinSource14,
		.GPIO_AF_Mapping = GPIO_AF_TIM12,
	},
	.FlashLight_Brightness = 1,//default brightness 1%
};

Device_FlashLight_t *Periph_FlashLight_Bottom = __VAR_CAST_VAR(Device_FlashLight_t)
{
	.FlashLight_Timer_PWM = NULL,
	.FlashLight_Timer_PWM_Channel = __CONST_CAST_VAR(HAL_Timer_PWM_Channel_t){
		.Timer_PWM_ChannelIdx = TIMER_PWM_CHANNEL_2,//CH2
		.Timer_PWM_Channel_OCInitCfg = __CONST_CAST_VAR(TIM_OCInitTypeDef)
		{
			.TIM_OCMode = TIM_OCMode_PWM1,
			.TIM_OutputState = TIM_OutputState_Enable,
			.TIM_OutputNState = TIM_OutputNState_Disable,
			.TIM_Pulse = 0, //default duty cycle to 0
			.TIM_OCPolarity = TIM_OCPolarity_High,//CNT > CCR, output will set high 
			//.TIM_OCNPolarity = TIM_OCNPolarity_High, TODO: check this value
			//.TIM_OCIdleState = TIM_OCIdleState_Reset,
			//.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
		}
	},
	.FlashLight_GPIO_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t) //PB15
	{
		.GPIOx = GPIOB,
		.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_AHB1Periph = RCC_AHB1Periph_GPIOB,
		},
		.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
			.GPIO_Pin = GPIO_Pin_15,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
		.GPIO_AF_PinSource = GPIO_PinSource15,
		.GPIO_AF_Mapping = GPIO_AF_TIM12,
	},
	.FlashLight_Brightness = 1,//default brightness 1%
};

//Unique ID
HAL_UniqueID_t *Periph_UniqueID = __CONST_CAST_VAR(HAL_UniqueID_t){
	.UniqueID_Addrs = __CONST_ARRAY_CAST_VAR(uint32_t){
		0x1FFF7A10, 0x1FFF7A14, 0x1FFF7A18
	},
	.UniqueID_Len = 3,
};

//Camera
HAL_MCO_t *Periph_MCO2_Cam = __CONST_CAST_VAR(HAL_MCO_t){
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
};

Device_CamOV2640_t _Cam_OV2640  = {
	.CamOV2640_DCMI = __VAR_CAST_VAR(HAL_DCMI_t){
		.DCMIx = DCMI,
		.DCMI_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_AHB2Periph = RCC_AHB2Periph_DCMI,
		},
		.DCMI_Pins = __CONST_ARRAY_CAST_VAR(HAL_GPIO_pin_t*){
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOC, //DCMI_D0 - PC6
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_6,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource6,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOC, //DCMI_D1 - PC7
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_7,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource7,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOE, //DCMI_D2 - PE0
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOE,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_0,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource0,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOE, //DCMI_D3 - PE1
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOE,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_1,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource1,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOE, //DCMI_D4 - PE4
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOE,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_4,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource4,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOB, //DCMI_D5 - PB6
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOB,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_6,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource6,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOE, //DCMI_D6 - PE5
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOE,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_5,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource5,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOE, //DCMI_D7 - PE6
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOE,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_6,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource6,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOA, //DCMI_HSYNC - PA4
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOA,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_4,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource4,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOA, //DCMI_PIXCLK - PA6
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOA,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_6,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource6,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			__CONST_CAST_VAR(HAL_GPIO_pin_t){
				.GPIOx = GPIOB, //DCMI_VSYNC - PB7
				.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
					.RCC_AHB1Periph = RCC_AHB1Periph_GPIOB,
				},
				.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
					.GPIO_Pin = GPIO_Pin_7,
					.GPIO_Mode = GPIO_Mode_AF,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_NOPULL
				},
				.GPIO_AF_PinSource = GPIO_PinSource7,
				.GPIO_AF_Mapping = GPIO_AF_DCMI,
			},
			NULL,//GPIO must null terminated
		},
		.DCMI_InitCfg = __CONST_CAST_VAR(DCMI_InitTypeDef){
			.DCMI_CaptureMode = DCMI_CaptureMode_SnapShot,
			.DCMI_SynchroMode = DCMI_SynchroMode_Hardware,
			.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising,
			.DCMI_VSPolarity = DCMI_VSPolarity_Low,
			.DCMI_HSPolarity = DCMI_HSPolarity_Low,
			.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame,
			.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b,
		},
		.DCMI_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef){
			.NVIC_IRQChannel = DCMI_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 2,
			.NVIC_IRQChannelSubPriority = 2,
			.NVIC_IRQChannelCmd = ENABLE,
		},
		.DCMI_RxDma_Cfg = __VAR_CAST_VAR(HAL_DMA_t){
			.DMA_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
				.RCC_AHB1Periph = RCC_AHB1Periph_DMA2,
			},
			.DMA_Streamx = DMA2_Stream1,
			.DMA_InitCfg = __CONST_CAST_VAR(DMA_InitTypeDef){
				.DMA_Channel = DMA_Channel_1,
				.DMA_PeripheralBaseAddr = 0,
				.DMA_Memory0BaseAddr = 0,
				.DMA_DIR = DMA_DIR_PeripheralToMemory,
				.DMA_BufferSize = 0, 
				.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word,
				.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word,
				.DMA_Mode = DMA_Mode_Normal,
				.DMA_Priority = DMA_Priority_High,
				.DMA_FIFOMode = DMA_FIFOMode_Enable,
				.DMA_FIFOThreshold = DMA_FIFOThreshold_Full,
				.DMA_MemoryBurst = DMA_MemoryBurst_INC4,
				.DMA_PeripheralBurst = DMA_PeripheralBurst_Single,
			},
			.DMA_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef){
				.NVIC_IRQChannel = DMA2_Stream1_IRQn,
				.NVIC_IRQChannelPreemptionPriority = 2,
				.NVIC_IRQChannelSubPriority = 2,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},
	},//dcmi
	.CamOV2640_I2C = __CONST_CAST_VAR(HAL_I2C_t){
		.I2Cx = I2C1,
		.I2C_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_APB1Periph = RCC_APB1Periph_I2C1,
		},
		.I2C_InitCfg = __CONST_CAST_VAR(I2C_InitTypeDef){
			.I2C_Mode = I2C_Mode_I2C,
			.I2C_DutyCycle = I2C_DutyCycle_2,
			.I2C_OwnAddress1 =0x0A, 
			.I2C_Ack = I2C_Ack_Enable,
			.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,	
			.I2C_ClockSpeed = 400000, //400k
		},
		.I2C_SCL_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
			.GPIOx = GPIOB, //I2C1_SCL - PB8
			.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
				.RCC_AHB1Periph = RCC_AHB1Periph_GPIOB,
			},
			.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
				.GPIO_Pin = GPIO_Pin_8,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_OType = GPIO_OType_OD,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
			.GPIO_AF_PinSource = GPIO_PinSource8,
			.GPIO_AF_Mapping = GPIO_AF_I2C1,
		},
		.I2C_SDA_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
			.GPIOx = GPIOB, //I2C1_SCL - PB9
			.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
				.RCC_AHB1Periph = RCC_AHB1Periph_GPIOB,
			},
			.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
				.GPIO_Pin = GPIO_Pin_9,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_OType = GPIO_OType_OD,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
			.GPIO_AF_PinSource = GPIO_PinSource9,
			.GPIO_AF_Mapping = GPIO_AF_I2C1,
		}
	},//I2C
	.CamOV2640_PWDN_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
		.GPIOx = GPIOE, //PWDN - PE2 - active high
		.GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
			.RCC_AHB1Periph = RCC_AHB1Periph_GPIOE,
		},
		.GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
			.GPIO_Pin = GPIO_Pin_2,
			.GPIO_Mode = GPIO_Mode_OUT,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL, //external pulldown
		},
	},//PWDN_Pin
	.CamOV2640_Buffer = NULL,
	.CamOV2640_Buffer_Len = 0,
	// .CamOV2640_Buffer = __VAR_ARRAY_CAST_VAR(uint8_t,1024*12){0},
	// .CamOV2640_Buffer_Len = 1024*12,
};
Device_CamOV2640_t *Periph_Cam_OV2640 = &_Cam_OV2640;
//TODO: Change decl format to upper

//Mjpegd
Mjpegd_t* Periph_Mjpegd = __VAR_CAST_VAR(Mjpegd_t){
    .Port = 8080,
    .FrameBuf = __VAR_CAST_VAR(Mjpegd_FrameBuf_t){
        ._frames_len = MJPEGD_FRAMEBUF_LEN,
        ._frames = __VAR_ARRAY_CAST_VAR(Mjpegd_Frame_t,MJPEGD_FRAMEBUF_LEN){
            0
        },
    },
    .Camera = __VAR_CAST_VAR(Mjpegd_Camera_t){
		.Ov2640_RecvRawFrame_cb = NULL,
        .HwCam_Ov2640 = &_Cam_OV2640,
    },
};//Periph_Mjpegd
