#include "stm32f4xx.h"


#include "bsp/platform/periph_list.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/timer.h"
#include "bsp/sys/sysctrl.h"
#include "stdio.h"

//Normally, you should not directly print stuff in ISR callback,
//because our buffer is not thread safe, this is just for test purpose
void Tx_Empty_Callback(void *sender,void* para)
{
	HAL_GPIO_TogglePin(LED_STAT_pin);
}
void Rx_ThrsReach_Callback(void *sender,void* para)
{ 
	uint8_t ch;
	HAL_USART_t* usart = (HAL_USART_t*)sender;
	printf("Rx_ThrsReach_Callback,in IRq %d\n",SysCtrl_IsThreadInIRq()); 
	printf("len %d, content:",Buffer_Queue_GetSize(usart->USART_Rx_Buf));
	while(Buffer_Queue_Pop_uint8_t(usart->USART_Rx_Buf,&ch))
		printf("%c",ch);
	printf("\n");
}
void Rx_Timeout_Callback(void *sender,void* para)
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
void Rx_Dropped_Callback(void *sender,void* para)
{
	printf("Rx_Dropped_Callback,in IRq %d\n",SysCtrl_IsThreadInIRq());
}
void Rx_Full_Callback(void *sender,void* para)
{
	printf("Rx_Full_Callback,in IRq %d\n",SysCtrl_IsThreadInIRq());
}

Callback_t USART_Tx_Empty_Callback = {Tx_Empty_Callback,NULL,INVOKE_IN_IRQ};
Callback_t USART_Rx_ThrsReach_Callback = {Rx_ThrsReach_Callback,NULL,INVOKE_IN_TASK};
Callback_t USART_Rx_Timeout_Callback = {Rx_Timeout_Callback,NULL,INVOKE_IN_TASK};
CallbackIRq_t USART_Rx_Dropped_Callback = {Rx_Dropped_Callback,NULL};
CallbackIRq_t USART_Rx_Full_Callback = {Rx_Full_Callback,NULL};
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
	Debug_Usart3->USART_Tx_Empty_Callback = &USART_Tx_Empty_Callback;
	Debug_Usart3->USART_Rx_ThrsReach_Callback = &USART_Rx_ThrsReach_Callback;
	Debug_Usart3->USART_Rx_Timeout_Callback = &USART_Rx_Timeout_Callback;
	Debug_Usart3->USART_Rx_Dropped_Callback = &USART_Rx_Dropped_Callback;
	Debug_Usart3->USART_Rx_Full_Callback = &USART_Rx_Full_Callback;

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
		printf("%c",ch++);
		if(ch>'z') 
		{	
			ch='a';
			printf("\n");
		} 

		//blink Load LED
		if(Timer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(LED_Load_pin);
			Timer_Reset(&blinkTimer);
			//printf("%d:Wkup pin %d\n",Systime_Get(),HAL_GPIO_ReadPin(Button_Wkup_pin));
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
