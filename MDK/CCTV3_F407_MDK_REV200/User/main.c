#include "bsp/platform/platform_defs.h"
#include "bsp/platform/platform_inst.h"

#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/systick.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/semaphore.h"
#include "bsp/hal/systick.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"
#include "bsp/hal/rtc.h"

//eth & lwip
#include "lwip/timeouts.h"
#include "eth/stm32f4x7_eth.h"
#include "eth/stm32f4x7_eth_phy.h"
#include "eth/netconf.h"

//apps
#include "lwip/apps/httpd.h"
#include "app/mjpegd/mjpegd.h"


SysTimer_t blinkTimer;

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

	//GPIO
	HAL_GPIO_InitPin(Peri_Button_Wkup_pin);
	HAL_GPIO_InitPin(Peri_LED_STAT_pin);
	HAL_GPIO_InitPin(Peri_LED_Load_pin);
	HAL_GPIO_WritePin(Peri_LED_STAT_pin,0);

	//PWM and flashlight
	HAL_Timer_PWM_Init(Periph_Timer_PWM_FlashLight);
	Device_FlashLight_Attach_PWM(Dev_FlashLight_Top,Periph_Timer_PWM_FlashLight);
	Device_FlashLight_Init(Dev_FlashLight_Top);
	Device_FlashLight_Attach_PWM(Dev_FlashLight_Bottom,Periph_Timer_PWM_FlashLight);
	Device_FlashLight_Init(Dev_FlashLight_Bottom);
	HAL_Timer_PWM_Cmd(Periph_Timer_PWM_FlashLight,true);

	//RNG
	HAL_Rng_Init(Peri_Rng);

	//Lwip & ETH & httpd
	ETH_BSP_Config();	
	LwIP_Init();
	httpd_init();
	NetTime_Init(App_NetTime);
	Mjpegd_Init(App_Mjpegd);
	
	SysTimer_Init(&blinkTimer,1000);
	while(1)
	{
		uint8_t rxcmd[DEBUG_SERIAL_RX_BUFFER_SIZE]={0};
		if(DBG_Serial_ReadLine(Peri_DBG_Serial,rxcmd,sizeof(rxcmd)))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
				DBG_INFO("Hello there\n");
			else if(strcmp((char*)rxcmd,"on")==0)
			{
				Device_FlashLight_Cmd(Dev_FlashLight_Top,true);
				Device_FlashLight_Cmd(Dev_FlashLight_Bottom,true);
				DBG_INFO("Flashlight on\n");
			}
			else if(strcmp((char*)rxcmd,"off")==0)
			{
				Device_FlashLight_Cmd(Dev_FlashLight_Top,false);
				Device_FlashLight_Cmd(Dev_FlashLight_Bottom,false);
				DBG_INFO("Flashlight off\n");
			}
			else if(strcmp((char*)rxcmd,"time")==0)
			{
				HAL_RTC_PrintTime(Peri_RTC);
			}
		}

		//blink Load LED
		if(SysTimer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(Peri_LED_Load_pin);
			SysTimer_Reset(&blinkTimer);
			//DBG_INFO("%d:Wkup pin %d\n",SysTime_Get(),HAL_GPIO_ReadPin(Periph_Button_Wkup_pin));
		}
		
		/* process received ethernet packet */
		while (ETH_CheckFrameReceived())
			LwIP_Pkt_Handle();
		/* handle periodic timers for LwIP */
		sys_check_timeouts();
	}
}
