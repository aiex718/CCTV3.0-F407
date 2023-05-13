#include "bsp/platform/platform_defs.h"
#include "bsp/platform/platform_inst.h"
#include "bsp/platform/platform_callbacks.h"

#include "bsp/sys/dbg_serial.h"

#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/mem_guard.h"

#include "bsp/hal/systick.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"
#include "bsp/hal/rtc.h"
#include "bsp/hal/adc.h"

//eth & lwip
#include "lwip/timeouts.h"
#include "bsp/eth/stm32f4x7_eth.h"
#include "bsp/eth/stm32f4x7_eth_phy.h"
#include "bsp/eth/netconf.h"

//apps
#include "lwip/apps/httpd.h"


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

	//Configs
	Config_Storage_Init(Dev_ConfigStorage);
	Config_Storage_Load(Dev_ConfigStorage);
	if(Config_Storage_IsChanged(Dev_ConfigStorage))
	{
		bool b = Config_Storage_Commit(Dev_ConfigStorage);
		DBG_INFO("Config_Storage_Commit %s\n",b?"OK":"Failed");
	}

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

	HAL_GPIO_InitPin(Peri_LED_STAT_pin);
	HAL_GPIO_WritePin(Peri_LED_STAT_pin,0);

	//Buzzer
	Device_Buzzer_Init(Dev_Buzzer);
	Device_Buzzer_ShortBeep(Dev_Buzzer);

	//PWM and flashlight
	HAL_Timer_PWM_Init(Periph_Timer_PWM_FlashLight);
	Device_FlashLight_Attach_PWM(Dev_FlashLight_Top,Periph_Timer_PWM_FlashLight);
	Device_FlashLight_Init(Dev_FlashLight_Top);
	Device_FlashLight_Attach_PWM(Dev_FlashLight_Bottom,Periph_Timer_PWM_FlashLight);
	Device_FlashLight_Init(Dev_FlashLight_Bottom);
	HAL_Timer_PWM_Cmd(Periph_Timer_PWM_FlashLight,true);

	//RNG
	HAL_Rng_Init(Peri_Rng);
	BSP_SRAND((uint16_t)HAL_Rng_Gen(Peri_Rng));

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

	//ADC
	HAL_ADC_CommonInit(Peri_ADC_CommonCfg);

	//Lwip & ETH 
	ETH_BSP_Config();	
	LwIP_Init();
	Mjpegd_Init(App_Mjpegd);
	NetTime_Init(App_NetTime);
	httpd_init();
	
	//Current trig
	Device_CurrentTrig_Init(Dev_CurrentTrig);
	if(Dev_CurrentTrig->CurrentTrig_Enable)
		Device_CurrentTrig_Cmd(Dev_CurrentTrig,true);
	else
		DBG_WARNING("CurrentTrig disabled\n");

	//Link callbacks handler to object
	Platform_RegistCallbacks();

	DBG_INFO("Init done\n");

	while(1)
	{
		DBG_Serial_Service(Peri_DBG_Serial);
		Device_Buzzer_Service(Dev_Buzzer);
    	Device_LedIndicator_Service(Dev_Led_Blink);
    	Device_Button_Service(Dev_Button_Wkup);
		Device_CurrentTrig_Service(Dev_CurrentTrig);
		UartCmd_Service(App_UartCmd);

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
