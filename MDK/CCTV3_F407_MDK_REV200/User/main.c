#include "bsp/platform/platform_defs.h"
#include "bsp/platform/platform_inst.h"

#include "bsp/sys/dbg_serial.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/hal/systick.h"
#include "bsp/sys/mem_guard.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/hal/systick.h"
#include "bsp/hal/rtc.h"


//eth & lwip
#include "lwip/timeouts.h"
#include "bsp/eth/stm32f4x7_eth.h"
#include "bsp/eth/stm32f4x7_eth_phy.h"
#include "bsp/eth/netconf.h"

//eth & lwip
#include "lwip/timeouts.h"
#include "bsp/eth/stm32f4x7_eth.h"
#include "bsp/eth/stm32f4x7_eth_phy.h"
#include "bsp/eth/netconf.h"

void Button_Wkup_ShortPress_Handler(void* sender, void* args,void* owner)
{
	DBG_INFO("Button_Wkup_ShortPress_Handler\n");
}

const Callback_t Button_Wkup_ShortPress_CB = 
{
	.func = Button_Wkup_ShortPress_Handler,
};

int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();
	delay(500); //wait 500ms for subsystems to be ready
	
	//RCC
	HAL_RCC_Init(Peri_RCC);

	//DBG_Serial using USART3
	DBG_Serial_Init(Peri_DBG_Serial);
	DBG_Serial_Cmd(Peri_DBG_Serial,true);
	DBG_INFO("Built at " __DATE__ " " __TIME__ " ,Booting...\n");
	
	//RTC Init
	HAL_RTC_Init(Peri_RTC);
	{
		uint32_t stack_size  = Mem_Guard_Init();
		DBG_INFO("Mem_Guard_Init stack size 0x%x\n",stack_size);
	}

	//Led Indicator
	Device_LedIndicator_Init(Dev_Led_Blink);
	//Button
	Device_Button_Init(Dev_Button_Wkup);
	//regist button callbacks
	Device_Button_SetCallback(Dev_Button_Wkup,BUTTON_CALLBACK_SHORT_PRESS,(Callback_t*)&Button_Wkup_ShortPress_CB);

	HAL_GPIO_InitPin(Peri_LED_STAT_pin);
	HAL_GPIO_WritePin(Peri_LED_STAT_pin,0);

	//RNG
	HAL_Rng_Init(Peri_Rng);
	BSP_SRAND((uint16_t)HAL_Rng_Gen(Peri_Rng));

	//Lwip & ETH 
	ETH_BSP_Config();	
	LwIP_Init();
	NetTime_Init(App_NetTime);
	
	while(1)
	{
		uint8_t rxcmd[DEBUG_SERIAL_RX_BUFFER_SIZE]={0};
		if(DBG_Serial_ReadLine(Peri_DBG_Serial,rxcmd,sizeof(rxcmd)))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
				DBG_INFO("Hello there\n");
			else if(strcmp((char*)rxcmd,"time")==0)
			{
				HAL_RTC_PrintTime(Peri_RTC);
			}
		}
		
		DBG_Serial_Service(Peri_DBG_Serial);
		Device_LedIndicator_Service(Dev_Led_Blink);
		Device_Button_Service(Dev_Button_Wkup);

		if(Mem_Guard_CheckOVF())
		{
			DBG_ERROR("Stack overflow detected\n");
			while(1);
		}
		
		/* process received ethernet packet */
		while (ETH_CheckFrameReceived())
			LwIP_Pkt_Handle();
		/* handle periodic timers for LwIP */
		sys_check_timeouts();
		SysCtrl_Service();
	}
}
