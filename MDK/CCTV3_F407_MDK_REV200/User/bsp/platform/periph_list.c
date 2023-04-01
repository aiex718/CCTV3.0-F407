#include "bsp/platform/periph_list.h"

#include "bsp/sys/concurrent_queue.h"
#include "bsp/sys/callback.h"
#include "bsp/hal/rcc.h"

#define __VAR_CAST_VAR(type) (type*)&(type)
#define __VAR_ARRAY_CAST_VAR(type,len) (type*)&(type[len])
#define __CONST_CAST_VAR(type) (type*)&(const type)
#define __CONST_ARRAY_CAST_VAR(type) (type*)&(const type[])

//GPIO LEDs
const HAL_GPIO_pin_t *LED_Load_pin = &(const HAL_GPIO_pin_t)
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

const HAL_GPIO_pin_t *LED_STAT_pin = &(const HAL_GPIO_pin_t)
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
const HAL_GPIO_pin_t *Button_Wkup_pin = &(const HAL_GPIO_pin_t)
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

//Debug serial
HAL_USART_t *Debug_Usart3 = &(HAL_USART_t)
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
		.DMA_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef)
		{
			.NVIC_IRQChannel = DMA1_Stream3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 2,
			.NVIC_IRQChannelSubPriority = 2,
			.NVIC_IRQChannelCmd = DISABLE,
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
		.DMA_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef)
		{
			.NVIC_IRQChannel = DMA1_Stream1_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 2,
			.NVIC_IRQChannelSubPriority = 2,
			.NVIC_IRQChannelCmd = DISABLE,
		},
	},
	.USART_Tx_Buf = __VAR_CAST_VAR(Buffer_uint8_t)
	{
		.buf_ptr = __VAR_ARRAY_CAST_VAR(uint8_t,Debug_Serial_Tx_Buffer_Size)
		{
			0
		},
		.len=Debug_Serial_Tx_Buffer_Size
	},
	.USART_Rx_Buf = __VAR_CAST_VAR(Buffer_uint8_t)
	{
		.buf_ptr = __VAR_ARRAY_CAST_VAR(uint8_t,Debug_Serial_Rx_Buffer_Size)
		{
			0
		},
		.len=Debug_Serial_Rx_Buffer_Size
	},
	.USART_Rx_Threshold = 0,
	.USART_Rx_Timeout = 100,
	.pExtension = NULL,
};

DBG_Serial_t* DBG_Serial= &(DBG_Serial_t)
{
	.tx_con_queue = __VAR_CAST_VAR(Concurrent_Queue_uint8_t)
	{
		.mem = __VAR_ARRAY_CAST_VAR(uint8_t,Debug_Serial_Tx_Buffer_Size)
		{
			0
		},
		.mem_len = Debug_Serial_Tx_Buffer_Size
	},
	.rx_buf = __VAR_CAST_VAR(Buffer_uint8_t)
	{
		.buf_ptr = __VAR_ARRAY_CAST_VAR(uint8_t,Debug_Serial_Rx_Buffer_Size)
		{
			0
		},
		.len=Debug_Serial_Rx_Buffer_Size
	},
	//alloc memory for callbacks
	._tx_empty_cb = __VAR_CAST_VAR(Callback_t){0},
	._rx_timeout_cb = __VAR_CAST_VAR(Callback_t){0},
};

HAL_Timer_PWM_t *Timer_PWM_FlashLight = &(HAL_Timer_PWM_t)
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
			.TIM_Period = 1000-1, //set duty cycle max to 1000, pwm freq = 1MHz/1000 = 1KHz
			.TIM_ClockDivision = TIM_CKD_DIV1,
			.TIM_RepetitionCounter = 0,
		},
		.Timer_NVIC_InitCfg = __CONST_CAST_VAR(NVIC_InitTypeDef){0},
	},
	.Timer_PWM_InitCfg = __CONST_CAST_VAR(TIM_OCInitTypeDef)
	{
		.TIM_OCMode = TIM_OCMode_PWM1,
		.TIM_OutputState = TIM_OutputState_Enable,
		.TIM_OutputNState = TIM_OutputNState_Disable,
		.TIM_Pulse = 100, //PWM default 100/1000 = 10% 
		.TIM_OCPolarity = TIM_OCPolarity_High,//CNT > CCR, output will set high 
		//.TIM_OCNPolarity = TIM_OCNPolarity_High, TODO: check this value
		//.TIM_OCIdleState = TIM_OCIdleState_Reset,
		//.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
	},
	.Timer_PWM_pins = 
	{
		__CONST_CAST_VAR(HAL_GPIO_pin_t) //CH0
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
		__CONST_CAST_VAR(HAL_GPIO_pin_t) //CH1
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
		NULL,//CH3 not available for TIM12
		NULL,//CH4 not available for TIM12
	}
};

