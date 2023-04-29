#include "bsp/platform/platform_defs.h"
#include "bsp/platform/platform_inst.h"

#include "bsp/sys/dbg_serial.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/hal/systick.h"
#include "bsp/sys/mem_guard.h"

//eth & lwip
#include "lwip/timeouts.h"
#include "bsp/eth/stm32f4x7_eth.h"
#include "bsp/eth/stm32f4x7_eth_phy.h"
#include "bsp/eth/netconf.h"

//apps
#include "lwip/apps/httpd.h"


SysTimer_t blinkTimer;

int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();

	delay(500); //wait 500ms for subsystems to be ready
	
	//DBG_Serial using USART3
	//DBG_Serial
	DBG_Serial_Init(Peri_DBG_Serial);
	DBG_Serial_Cmd(Peri_DBG_Serial,true);
	DBG_INFO("Built at " __DATE__ " " __TIME__ " ,Booting...\n");
	
	{
		uint32_t stack_size  = Mem_Guard_Init();
		DBG_INFO("Mem_Guard_Init stack size 0x%x\n",stack_size);
	}


	//GPIO
	HAL_GPIO_InitPin(Peri_Button_Wkup_pin);
	HAL_GPIO_InitPin(Peri_LED_STAT_pin);
	HAL_GPIO_InitPin(Peri_LED_Load_pin);
	HAL_GPIO_WritePin(Peri_LED_STAT_pin,0);

	//RNG
	HAL_Rng_Init(Peri_Rng);

	//Lwip & ETH & httpd
	ETH_BSP_Config();	
	LwIP_Init();
	httpd_init();
	
	SysTimer_Init(&blinkTimer,1000);
	while(1)
	{
		uint8_t rxcmd[DEBUG_SERIAL_RX_BUFFER_SIZE]={0};
		if(DBG_Serial_ReadLine(Peri_DBG_Serial,rxcmd,BSP_ARR_LEN(rxcmd)))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
				DBG_INFO("Hello there\n");
		}
		
		DBG_Serial_Service(Peri_DBG_Serial);
		//blink Load LED
		if(SysTimer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(Peri_LED_Load_pin);
			SysTimer_Reset(&blinkTimer);
				DBG_INFO("%d:Wkup pin %d\n",SysTime_Get(),HAL_GPIO_ReadPin(Peri_Button_Wkup_pin));
		}

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
	}
}
