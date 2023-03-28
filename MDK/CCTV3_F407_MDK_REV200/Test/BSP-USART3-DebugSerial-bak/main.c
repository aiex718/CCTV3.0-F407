#include "stm32f4xx.h"

#include "bsp/platform/periph_list.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/timer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/dbg_serial.h"

Timer_t blinkTimer;
char ch='a';
bool StressTestTx=false;
int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();
	//USART3 for debug
	HAL_USART_Init(Debug_Usart3);
	DBG_Serial_Init(DBG_Serial);
	DBG_Serial_AttachUSART(DBG_Serial,Debug_Usart3);
	HAL_USART_Cmd(Debug_Usart3,true);


	HAL_GPIO_InitPin(Button_Wkup_pin);
	HAL_GPIO_InitPin(LED_STAT_pin);
	HAL_GPIO_InitPin(LED_Load_pin);
	HAL_GPIO_WritePin(LED_STAT_pin,0);
	
	printf("Boot\n");

	
	Timer_Init(&blinkTimer,1000);
	while(1)
	{
		uint8_t rxcmd[16]={0};
		if(DBG_Serial_ReadLine(DBG_Serial,rxcmd,16))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
			{
				printf("hello there\n");
			}
			else if(strcmp((char*)rxcmd,"stress")==0)
			{
				StressTestTx=true;
				printf("Stress test started\n");
			}
			else if(strcmp((char*)rxcmd,"stop")==0)
			{
				StressTestTx=false;
				printf("Stress test stopped\n");
			}
			else if(strcmp((char*)rxcmd,"reset")==0)
			{
				printf("Resetting...\n");
				SysCtrl_Reset();
			}
			else if(strcmp((char*)rxcmd,"help")==0)
			{
				printf("Commands:\n");
				printf("stress - start stress test\n");
				printf("stop - stop stress test\n");
				printf("reset - reset the board\n");
				printf("hello - echo hello\n");
				printf("help - print this help\n");
			}
			else
			{
				printf("Unknown command:%s\n",rxcmd);
			}
		}
		if(StressTestTx)
		{
			printf("%c",ch++);
			if(ch>'z') 
			{	
				ch='a';
				printf("\n %d:",Systime_Get());
			} 
		}



		HAL_USART_Service(Debug_Usart3);
		DBG_Serial_Service(DBG_Serial);

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


