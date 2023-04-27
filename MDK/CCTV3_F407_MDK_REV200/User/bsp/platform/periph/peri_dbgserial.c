#include "bsp/platform/periph/peri_dbgserial.h"

#include "bsp/sys/concurrent_queue.h"
#include "bsp/hal/usart.h"
#include "bsp/sys/callback.h"

DBG_Serial_t Peri_DBG_Serial_Inst= 
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
			.buf_ptr = __VAR_ARRAY_CAST_VAR(uint8_t,DEBUG_SERIAL_USART_TX_BUFFER_SIZE)
			{
				0
			},
			.len=DEBUG_SERIAL_USART_TX_BUFFER_SIZE
		},
		.USART_Rx_Buf = __VAR_CAST_VAR(Buffer_uint8_t)
		{
			.buf_ptr = __VAR_ARRAY_CAST_VAR(uint8_t,DEBUG_SERIAL_USART_RX_BUFFER_SIZE)
			{
				0
			},
			.len=DEBUG_SERIAL_USART_RX_BUFFER_SIZE
		},
		.USART_Rx_Threshold = 0,
		.USART_Rx_Timeout = 100,
		.pExtension = NULL,
	},//hal_usart
};

DBG_Serial_t* Peri_DBG_Serial=&Peri_DBG_Serial_Inst;
