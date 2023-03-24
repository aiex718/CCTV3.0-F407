#include "stm32f4xx.h"


#include "bsp/platform/periph_list.h"
#include "bsp/sys/systime.h"


void USARTx_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
		
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8  ;  
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_USART3);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure); 

	/* 使能串口 */
	USART_Cmd(USART3, ENABLE);
}




int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();
	//USARTx_Config();
	HAL_GPIO_InitPin(Button_Wkup_pin);
	HAL_GPIO_InitPin(LED_STAT_pin);
	HAL_GPIO_InitPin(LED_Load_pin);
	HAL_USART_Init(Debug_Usart3);
	HAL_USART_Cmd(Debug_Usart3,true);
	HAL_USART_RxStreamCmd(Debug_Usart3,true);

	printf("hello world!\n");

	while(1)
	{
		uint8_t ch;
		HAL_USART_Service(Debug_Usart3);
		while(HAL_USART_ReadByte(Debug_Usart3, &ch))
		{
			HAL_USART_WriteByte(Debug_Usart3, ch);
		}

		HAL_GPIO_WritePin(LED_STAT_pin,1);
		HAL_GPIO_WritePin(LED_Load_pin,0);
		//printf("%d:pin %d\n",Systime_Get(),HAL_GPIO_ReadPin(Button_Wkup_pin));
		delay(1000);

		HAL_GPIO_WritePin(LED_STAT_pin,0);
		HAL_GPIO_WritePin(LED_Load_pin,1);
		//printf("%d:pin %d\n",Systime_Get(),HAL_GPIO_ReadPin(Button_Wkup_pin));
		delay(1000);
		
	}
}
