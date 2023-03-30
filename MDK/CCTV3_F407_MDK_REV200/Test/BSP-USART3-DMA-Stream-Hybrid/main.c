#include "stm32f4xx.h"


#include "bsp/platform/periph_list.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/timer.h"
#include "bsp/sys/sysctrl.h"
#include "stdio.h"

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
	uint16_t cnt=usart->USART_Rx_Threshold,i=0;
	uint8_t rx_data[Debug_Serial_Rx_Buffer_Size]={0};
	while (i<cnt)
	{
		Buffer_Queue_Pop_uint8_t(rx_queue,rx_data+i);
		i++;
	}
	
	printf("Rx_ThrsReach_Callback,in IRq %d ,len:%d ,content: %s\n",
		SysCtrl_IsThreadInIRq(),cnt,rx_data);
}

void Rx_Timeout(void *sender,void* para)
{	
	uint8_t ch;
	bool IsRxStreamEnabled=HAL_USART_IsRxStreamEnabled(Debug_Usart3);
	HAL_USART_t* usart = (HAL_USART_t*)sender;
	Buffer_uint8_t *tx_queue=usart->USART_Tx_Buf;

	while (HAL_USART_IsTransmitting(usart));// wait until tx finished
	Buffer_Clear(tx_queue);
	tx_queue->w_ptr+=sprintf(tx_queue->w_ptr,(IsRxStreamEnabled ? "Rx from stream, ":"Rx from dma, "));
	tx_queue->w_ptr+=sprintf(tx_queue->w_ptr,"Rx_Timeout_Callback,in IRq %d ,",SysCtrl_IsThreadInIRq());
	tx_queue->w_ptr+=sprintf(tx_queue->w_ptr,"len %d, content:",Buffer_Queue_GetSize(usart->USART_Rx_Buf));
	while(Buffer_Queue_Pop_uint8_t(usart->USART_Rx_Buf,&ch))
		Buffer_Queue_Push_uint8_t(tx_queue,ch);
	tx_queue->w_ptr+=sprintf(tx_queue->w_ptr,"\n");

	//switch rx between DMA and Stream(interrupt) mode
	if(IsRxStreamEnabled)
	{
		HAL_USART_RxStreamCmd(Debug_Usart3,false);
		HAL_USART_DmaRead(Debug_Usart3,0);
	}
	else
	{
		HAL_USART_RxStreamCmd(Debug_Usart3,true);
	}
	
	HAL_USART_DmaWrite(usart);
}
//Becareful these callback is always executed in usart ISR, and printf will block until tx finished
//so if tx buffer is full, thread will hang forever , this is just for demo purpose
//DONT PRINTF IN ISR UNLESS YOU IMPLEMENT NON-BLOCKING PUTC 
void Rx_Dropped(void *sender,void* para)
{
	printf("RxDrop\n");
}
void Rx_Full(void *sender,void* para)
{
	printf("RxFull\n");
}

Callback_t Tx_Empty_Callback = {Tx_Empty,NULL,INVOKE_IN_IRQ};
Callback_t Rx_ThrsReach_Callback = {Rx_ThrsReach,NULL,INVOKE_IN_TASK};
Callback_t Rx_Timeout_Callback = {Rx_Timeout,NULL,INVOKE_IN_TASK};
Callback_t Rx_Dropped_Callback = {Rx_Dropped,NULL,INVOKE_IN_IRQ};
Callback_t Rx_Full_Callback = {Rx_Full,NULL,INVOKE_IN_IRQ};

Timer_t blinkTimer;
int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();
	HAL_GPIO_InitPin(Button_Wkup_pin);
	HAL_GPIO_InitPin(LED_STAT_pin);
	HAL_GPIO_InitPin(LED_Load_pin);
	HAL_USART_Init(Debug_Usart3);
	HAL_GPIO_WritePin(LED_STAT_pin,0);

	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_TX_EMPTY,&Tx_Empty_Callback);
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_RX_THRSHOLD,&Rx_ThrsReach_Callback);
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_RX_TIMEOUT,&Rx_Timeout_Callback);
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_IRQ_RX_DROPPED,&Rx_Dropped_Callback);
	HAL_USART_SetCallback(Debug_Usart3,USART_CALLBACK_IRQ_RX_FULL,&Rx_Full_Callback);

	HAL_USART_Cmd(Debug_Usart3,true);
	HAL_USART_DmaRead(Debug_Usart3,0);

	printf("Hello World from tx stream!\n");
	while (HAL_USART_IsTransmitting(Debug_Usart3));// wait until tx finished

	{
		Buffer_uint8_t *tx_queue=Debug_Usart3->USART_Tx_Buf;
		Buffer_Clear(tx_queue);
		uint16_t w_len = sprintf(tx_queue->w_ptr,"Hello World from DMA! \n");
		tx_queue->w_ptr += w_len;
		HAL_USART_DmaWrite(Debug_Usart3);
	}
	
	//tx dma ongoing, this should fail
	HAL_USART_Write(Debug_Usart3,
		"2nd Hello World from tx stream, you should not see me!\n",
		strlen("2nd Hello World from tx stream, you should not see me!\n"));

	//this will block thread until DMA tx finished
	printf("3rd Hello World from tx stream!\n");
	
	Timer_Init(&blinkTimer,1000);

	while(1)
	{
		HAL_USART_Service(Debug_Usart3);

		//blink Load LED
		if(Timer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(LED_Load_pin);
			Timer_Reset(&blinkTimer);
			printf("%d:Wkup pin %d\n",Systime_Get(),HAL_GPIO_ReadPin(Button_Wkup_pin));
		}
	}
}

int fputc(int ch, FILE *f)
{
	while (HAL_USART_WriteByte(Debug_Usart3, (uint8_t) ch)==false && SysCtrl_IsThreadInIRq() ==false);
	return (ch);
}

int fgetc(FILE *f)
{
	uint8_t ch;
	while (HAL_USART_ReadByte(Debug_Usart3, &ch)==false);
	return (int)ch;
}
