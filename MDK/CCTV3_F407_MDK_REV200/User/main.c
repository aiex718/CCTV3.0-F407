#include "stm32f4xx.h"

#include "bsp/platform/periph_list.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/timer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/dbg_serial.h"
#include "stdio.h"

Timer_t blinkTimer;
int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();
	//USART3 for debug
	HAL_USART_Init(Debug_Usart3);
	DBG_Serial_Init(DBG_Serial);
	DBG_Serial_AttachUSART(DBG_Serial,Debug_Usart3);
	HAL_USART_Cmd(Debug_Usart3,true);
	printf("Built at " __DATE__ " " __TIME__ " ,Booting...\n");

	HAL_GPIO_InitPin(Button_Wkup_pin);
	HAL_GPIO_InitPin(LED_STAT_pin);
	HAL_GPIO_InitPin(LED_Load_pin);
	HAL_GPIO_WritePin(LED_STAT_pin,0);
	

	
	Timer_Init(&blinkTimer,1000);
	while(1)
	{
		uint8_t rxcmd[Debug_Serial_Rx_Buffer_Size]={0};
		if(DBG_Serial_ReadLine(DBG_Serial,rxcmd,16))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
				printf("hello there\n");
		}

		HAL_USART_Service(Debug_Usart3);
		DBG_Serial_Service(DBG_Serial);

		//blink Load LED
		if(Timer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(LED_Load_pin);
			Timer_Reset(&blinkTimer);
				printf("%d:Wkup pin %d\n",Systime_Get(),HAL_GPIO_ReadPin(Button_Wkup_pin));
		}
	}
}


