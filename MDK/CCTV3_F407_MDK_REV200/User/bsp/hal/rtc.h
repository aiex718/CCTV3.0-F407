#ifndef RTC_H
#define RTC_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"

#include "time.h"

typedef struct
{
    HAL_RCC_Cmd_t *RTC_RCC_Cmd;
    RTC_InitTypeDef *RTC_InitCfg;
    uint32_t RTC_ClockSource;

    uint16_t RTC_Year_Offset;
    uint8_t RTC_Hour_Offset;
    uint8_t RTC_Minute_Offset;
    uint8_t RTC_WeekDay;
} HAL_RTC_t;

void HAL_RTC_Init(HAL_RTC_t *self);
time_t HAL_RTC_GetTime(HAL_RTC_t *self);
void HAL_RTC_GetTime_tm(HAL_RTC_t *self,struct tm *ptm);
void HAL_RTC_SetTime_tm(HAL_RTC_t *self,const struct tm *ptm);
void HAL_RTC_PrintTime(HAL_RTC_t *self);
#endif
