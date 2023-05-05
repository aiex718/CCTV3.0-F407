#include "bsp/platform/platform_defs.h"
#include "bsp/platform/platform_inst.h"

#include "bsp/sys/dbg_serial.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/mem_guard.h"

#include "bsp/hal/systick.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"
#include "bsp/hal/rtc.h"

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
	//TODO: use rand
	HAL_Rng_Init(Peri_Rng);

	//Disk
	Disk_InitAll(Dev_Disk_list);

	//USB
	USBOTG_fs_Init(Dev_USBOTG_fs);

	//FileSys
	FileSys_Init(App_FileSys);
	if(FileSys_Mount(App_FileSys,""))
		DBG_INFO("FileSys_Mount success\n");
	else
		DBG_ERROR("FileSys_Mount failed\n");

	//Lwip & ETH 
	ETH_BSP_Config();	
	LwIP_Init();
	Mjpegd_Init(App_Mjpegd);
	NetTime_Init(App_NetTime);
	httpd_init();
	
	//ADC
	HAL_ADC_CommonInit(Peri_ADC_CommonCfg);

	//Current trig
	//TODO:Regist triggered callback and webhook
	Device_CurrentTrig_Init(Dev_CurrentTrig);
	Device_CurrentTrig_Cmd(Dev_CurrentTrig,true);
	


	SysTimer_Init(&blinkTimer,1000);
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
			else if(strcmp((char*)rxcmd,"stack")==0)
			{
				DBG_INFO("Stack usage 0x%x\n",Mem_Guard_GetStackDepth());
			}
			else if(strcmp((char*)rxcmd,"format")==0)
			{
				if(FileSys_Format(App_FileSys,""))
					DBG_INFO("FileSys_Format success\n");
				else
					DBG_ERROR("FileSys_Format failed\n");
			}
		}

		//blink Load LED
		if(SysTimer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(Peri_LED_Load_pin);
			SysTimer_Reset(&blinkTimer);
			//DBG_INFO("%d:Wkup pin %d\n",SysTime_Get(),HAL_GPIO_ReadPin(Peri_Button_Wkup_pin));
		}

		Device_CurrentTrig_Service(Dev_CurrentTrig);

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
