#include "bsp/platform/periph/peri_rtc.h"

const HAL_RTC_t Peri_RTC_Inst = {
	.RTC_RCC_Cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t)
	{
		.RCC_APB1Periph = RCC_APB1Periph_PWR,
	},
	.RTC_InitCfg = __CONST_CAST_VAR(RTC_InitTypeDef)
	{
		//LSE = 32.768KHz
		//ck_spare = LSE/[(255+1)*(127+1)] = 1HZ
		.RTC_HourFormat = RTC_HourFormat_24,
		.RTC_AsynchPrediv = 0x7F,
		.RTC_SynchPrediv = 0xFF,
	},
	.RTC_ClockSource = RCC_RTCCLKSource_LSE,
	//RTC can only save 100 years, offset 2000 to 1900 by 100 years
	.RTC_Year_Offset = 100,//1900 to 2000
	.RTC_Hour_Offset = 0,
	.RTC_Minute_Offset = 0,
	.RTC_WeekDay = RTC_Weekday_Sunday,
};
const HAL_RTC_t *Peri_RTC = &Peri_RTC_Inst;
