#include "stm32f4xx.h"
#include "stdio.h"

#include "bsp/platform/periph_list.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/semaphore.h"
#include "bsp/hal/systick.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"

//eth & lwip
#include "lwip/timeouts.h"
#include "eth/stm32f4x7_eth.h"
#include "eth/stm32f4x7_eth_phy.h"
#include "eth/netconf.h"

//apps
#include "lwip/apps/httpd.h"
#include "eth/apps/mjpeg/mjpegd.h"


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
	
	HAL_GPIO_InitPin(Button_Wkup_pin);
	HAL_GPIO_InitPin(LED_STAT_pin);
	HAL_GPIO_InitPin(LED_Load_pin);
	HAL_GPIO_WritePin(LED_STAT_pin,0);

	//PWM and flashlight
	HAL_Timer_PWM_Init(Timer_PWM_FlashLight);
	Device_FlashLight_Attach_PWM(FlashLight_Top,Timer_PWM_FlashLight);
	Device_FlashLight_Init(FlashLight_Top);
	Device_FlashLight_Cmd(FlashLight_Top,true);
	Device_FlashLight_Attach_PWM(FlashLight_Bottom,Timer_PWM_FlashLight);
	Device_FlashLight_Init(FlashLight_Bottom);
	Device_FlashLight_Cmd(FlashLight_Bottom,true);
	HAL_Timer_PWM_Cmd(Timer_PWM_FlashLight,true);

	//Camera init
	HAL_MCO_Init(MCO2_Cam);
	Device_CamOV2640_Init(Cam_OV2640);
	
	//Lwip & ETH & httpd
	ETH_BSP_Config();	
	LwIP_Init();
	httpd_init();
	Mjpegd_Init(mjpeg_inst);
	
	SysTimer_Init(&blinkTimer,1000);
	while(1)
	{
		uint8_t rxcmd[DEBUG_SERIAL_RX_BUFFER_SIZE]={0};
		if(DBG_Serial_ReadLine(DBG_Serial,rxcmd,sizeof(rxcmd)))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
				DBG_INFO("Hello there\n");
			else if(strcmp((char*)rxcmd,"on")==0)
			{
				Device_FlashLight_Cmd(FlashLight_Top,true);
				Device_FlashLight_Cmd(FlashLight_Bottom,true);
				DBG_INFO("Flashlight on\n");
			}
			else if(strcmp((char*)rxcmd,"off")==0)
			{
				Device_FlashLight_Cmd(FlashLight_Top,false);
				Device_FlashLight_Cmd(FlashLight_Bottom,false);
				DBG_INFO("Flashlight off\n");
			}
		}

		//blink Load LED
		if(SysTimer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(LED_Load_pin);
			SysTimer_Reset(&blinkTimer);
			//DBG_INFO("%d:Wkup pin %d\n",SysTime_Get(),HAL_GPIO_ReadPin(Button_Wkup_pin));
		}
		
		/* process received ethernet packet */
		while (ETH_CheckFrameReceived())
			LwIP_Pkt_Handle();
		/* handle periodic timers for LwIP */
		sys_check_timeouts();
	}
}
