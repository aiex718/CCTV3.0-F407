#include "stm32f4xx.h"
#include "stdio.h"

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

int fputc(int ch, FILE *f)
{
	USART_SendData(USART3, (uint8_t) ch);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	return (ch);
}

int fgetc(FILE *f)
{
	while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);
	return (int)USART_ReceiveData(USART3);
}


int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	//SystemInit() will config sysclk to 180MHZ
	char rxBuffer[100];
	USARTx_Config();

	printf("hello world!");

	while(1)
	{
		scanf("%s",rxBuffer);
		printf("You say:%s",rxBuffer);
	}
}


