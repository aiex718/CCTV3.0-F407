#include "stm32f4xx.h"
#include "stdio.h"

#include "bsp/platform/periph_list.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/hal/systick.h"

//eth & lwip
#include "lwip/timeouts.h"
#include "eth/stm32f4x7_eth.h"
#include "eth/stm32f4x7_eth_phy.h"
#include "eth/netconf.h"

//apps
#include "lwip/apps/httpd.h"


SysTimer_t blinkTimer;

int main(void)
{
	//SystemInit() is inside system_stm32f4xx.c
	HAL_Systick_Init();
	delay(500); //wait 500ms for subsystems to be ready
	//USART3 for DBG_Serial
	DBG_Serial_Init(DBG_Serial);
	DBG_Serial_Cmd(DBG_Serial,true);
	DBG_INFO("Built at " __DATE__ " " __TIME__ " ,Booting...\n");

	//Lwip & ETH & httpd
	ETH_BSP_Config();	
	LwIP_Init();
	httpd_init();

	DBG_INFO("Webapi minimum build\n");
	DBG_INFO("Only /api?cmd=uptime implemented\n");
	
	SysTimer_Init(&blinkTimer,1000);
	while(1)
	{
		uint8_t rxcmd[DEBUG_SERIAL_RX_BUFFER_SIZE]={0};
		if(DBG_Serial_ReadLine(DBG_Serial,rxcmd,sizeof(rxcmd)))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
				DBG_INFO("Hello there\n");
		}
		
		/* process received ethernet packet */
		while (ETH_CheckFrameReceived())
			LwIP_Pkt_Handle();
		/* handle periodic timers for LwIP */
		sys_check_timeouts();
	}
}
