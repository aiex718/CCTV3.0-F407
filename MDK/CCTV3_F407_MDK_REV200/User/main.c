#include "stm32f4xx.h"

#include "bsp/platform/periph_list.h"
#include "bsp/hal/systick.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/mem_guard.h"
#include "bsp/sys/dbg_serial.h"
#include "stdio.h"

SysTimer_t blinkTimer;
int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();
	//DBG_Serial
	DBG_Serial_Init(DBG_Serial);
	DBG_Serial_Cmd(DBG_Serial,true);
	DBG_INFO("Built at " __DATE__ " " __TIME__ " ,Booting...\n");
	DBG_INFO("Mem_Guard_Init stack size 0x%x\n",Mem_Guard_Init());

	HAL_GPIO_InitPin(Periph_Button_Wkup_pin);
	HAL_GPIO_InitPin(Periph_LED_STAT_pin);
	HAL_GPIO_InitPin(Periph_LED_Load_pin);
	HAL_GPIO_WritePin(Periph_LED_STAT_pin,0);

	SysTimer_Init(&blinkTimer,1000);
	while(1)
	{
		uint8_t rxcmd[DEBUG_SERIAL_RX_BUFFER_SIZE]={0};
		if(DBG_Serial_ReadLine(DBG_Serial,rxcmd,BSP_ARR_LEN(rxcmd)))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
				DBG_INFO("hello there\n");
		}
		
		DBG_Serial_Service(DBG_Serial);
		//blink Load LED
		if(SysTimer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(Periph_LED_Load_pin);
			SysTimer_Reset(&blinkTimer);
				DBG_INFO("%d:Wkup pin %d\n",SysTime_Get(),HAL_GPIO_ReadPin(Periph_Button_Wkup_pin));
		}

		if(Mem_Guard_CheckOVF())
		{
			DBG_ERROR("Stack overflow detected\n");
			while(1);
		}
	}
}


