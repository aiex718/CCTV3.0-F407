#include "stm32f4xx.h"
#include "stdio.h"

#include "bsp/platform/periph_list.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/systimer.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/systick.h"
#include "bsp/hal/timer.h"
#include "bsp/hal/timer_pwm.h"


SysTimer_t blinkTimer;
SysTimer_t breathLightTimer;
#define DUTY_MAX 300

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

	//PWM and flashlight
	HAL_Timer_PWM_Init(Timer_PWM_FlashLight);
	Device_FlashLight_Attach(FlashLight_Top,Timer_PWM_FlashLight);
	Device_FlashLight_Attach(FlashLight_Bottom,Timer_PWM_FlashLight);
	Device_FlashLight_Cmd(FlashLight_Top,true);
	Device_FlashLight_Cmd(FlashLight_Bottom,true);
	
	
	Timer_Init(&blinkTimer,1000);
	Timer_Init(&breathLightTimer,10);
	while(1)
	{
		uint8_t rxcmd[Debug_Serial_Rx_Buffer_Size]={0};
		if(DBG_Serial_ReadLine(DBG_Serial,rxcmd,16))
		{
			if(strcmp((char*)rxcmd,"hello")==0)
				printf("hello there\n");
			else if(strcmp((char*)rxcmd,"on")==0)
			{
				Device_FlashLight_Cmd(FlashLight_Top,true);
				Device_FlashLight_Cmd(FlashLight_Bottom,true);
				printf("flash light on\n");
			}
			else if(strcmp((char*)rxcmd,"off")==0)
			{
				Device_FlashLight_Cmd(FlashLight_Top,false);
				Device_FlashLight_Cmd(FlashLight_Bottom,false);
				printf("flash light off\n");
			}
		}

		HAL_USART_Service(Debug_Usart3);
		DBG_Serial_Service(DBG_Serial);

		//blink Load LED
		if(SysTimer_IsElapsed(&blinkTimer))
		{
			HAL_GPIO_TogglePin(LED_Load_pin);
			SysTimer_Reset(&blinkTimer);
				printf("%d:Wkup pin %d\n",Systime_Get(),HAL_GPIO_ReadPin(Button_Wkup_pin));
		}

		//Breath light
		if(SysTimer_IsElapsed(&breathLightTimer))
		{
			static uint16_t duty_cycle=0;
			static int8_t step=1;
			duty_cycle+=step;
			if(duty_cycle>=DUTY_MAX)
			{
				duty_cycle=DUTY_MAX;
				step=-1;
			}
			else if(duty_cycle<=0)
			{
				duty_cycle=0;
				step=1;
			}
			Device_FlashLight_SetDutyCycle(FlashLight_Top,duty_cycle);
			Device_FlashLight_SetDutyCycle(FlashLight_Bottom,DUTY_MAX-duty_cycle);
			SysTimer_Reset(&breathLightTimer);
		}
	}
}


