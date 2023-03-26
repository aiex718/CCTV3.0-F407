#include "stm32f4xx.h"


#include "bsp/platform/periph_list.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/timer.h"
#include "bsp/sys/sysctrl.h"
#include "stdio.h"

bool StressTestTx=false;
//Normally, you should not directly print stuff in ISR callback,
//because our buffer is not thread safe, this is just for test purpose
void Tx_Empty(void *sender,void* para)
{
	HAL_GPIO_TogglePin(LED_STAT_pin);
}
void Rx_ThrsReach(void *sender,void* para)
{ 
	//because rx_buf could receive data anytime,
	//so we should snap buf size or use usart->USART_Rx_Threshold as rx count
	//instead of while(Buffer_Queue_Pop_uint8_t)
	HAL_USART_t* usart = (HAL_USART_t*)sender;
	Buffer_uint8_t *rx_queue=usart->USART_Rx_Buf;
	uint8_t rx_data[Debug_Serial_Rx_Buffer_Size]={0};
	uint16_t cnt=usart->USART_Rx_Threshold,i=0;
	while (i<cnt)
	{
		Buffer_Queue_Pop_uint8_t(rx_queue,rx_data+i);
		i++;
	}
	if(strcmp(rx_data,"stress")==0)
		StressTestTx=!StressTestTx;
	printf("Rx_ThrsReach_Callback,in IRq %d\n",SysCtrl_IsThreadInIRq()); 
	printf("len %d, content: %s\n",cnt,rx_data);
}
void Rx_Timeout(void *sender,void* para)
{	
	uint8_t ch;
	HAL_USART_t* usart = (HAL_USART_t*)sender;
	printf("Rx_Timeout_Callback,in IRq %d ,",SysCtrl_IsThreadInIRq());
	printf("len %d, content:",Buffer_Queue_GetSize(usart->USART_Rx_Buf));
	while(Buffer_Queue_Pop_uint8_t(usart->USART_Rx_Buf,&ch))
		printf("%c",ch);
	printf("\n");
}
//Config rx threshold to larger than rx_queue max capacity to test these callback
//Becareful these callback is executed in ISR, and printf will block until tx finished
//so if tx buffer is full, it will block the ISR, thread will hang forever
void Rx_Dropped(void *sender,void* para)
{
	printf("Rx_Dropped_Callback,in IRq %d\n",SysCtrl_IsThreadInIRq());
}
void Rx_Full(void *sender,void* para)
{
	printf("Rx_Full_Callback,in IRq %d\n",SysCtrl_IsThreadInIRq());
}

Callback_t Tx_Empty_Callback = {Tx_Empty,NULL,INVOKE_IN_IRQ};
Callback_t Rx_ThrsReach_Callback = {Rx_ThrsReach,NULL,INVOKE_IN_TASK};
Callback_t Rx_Timeout_Callback = {Rx_Timeout,NULL,INVOKE_IN_TASK};
Callback_t Rx_Dropped_Callback = {Rx_Dropped,NULL};
Callback_t Rx_Full_Callback = {Rx_Full,NULL};
Timer_t blinkTimer;
char ch='a';
int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();
	HAL_GPIO_InitPin(Button_Wkup_pin);
	HAL_GPIO_InitPin(LED_STAT_pin);
	HAL_GPIO_InitPin(LED_Load_pin);
	HAL_USART_Init(Debug_Usart3);
	
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_TX_EMPTY,&Tx_Empty_Callback);
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_RX_THRSHOLD,&Rx_ThrsReach_Callback);
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_RX_TIMEOUT,&Rx_Timeout_Callback);
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_IRQ_RX_DROPPED,&Rx_Dropped_Callback);
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_IRQ_RX_FULL,&Rx_Full_Callback);

	HAL_USART_Cmd(Debug_Usart3,true);
	HAL_USART_RxStreamCmd(Debug_Usart3,true);

	printf("hello world!\n");
	Timer_Init(&blinkTimer,1000);
	HAL_GPIO_WritePin(LED_STAT_pin,0);

	while(1)
	{
		HAL_USART_Service(Debug_Usart3);

		//stress test tx queue
		//could overwhalm your PC if it's a potato :P
		if(StressTestTx)
		{
			printf("%c",ch++);
			if(ch>'z') 
			{	
				ch='a';
				printf("\n");
			} 
		}

		//blink Load LED
		if(Timer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(LED_Load_pin);
			Timer_Reset(&blinkTimer);
			if(StressTestTx==false)
				printf("%d:Wkup pin %d\n",Systime_Get(),HAL_GPIO_ReadPin(Button_Wkup_pin));
		}
		
	}
}

int fputc(int ch, FILE *f)
{
	while (HAL_USART_WriteByte(Debug_Usart3, (uint8_t) ch)==false);
	return (ch);
}

int fgetc(FILE *f)
{
	uint8_t ch;
	while (HAL_USART_ReadByte(Debug_Usart3, &ch)==false);
	return (int)ch;
}
