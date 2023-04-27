#include "bsp/hal/rtc.h"
#include "bsp/sys/dbg_serial.h"

void HAL_RTC_Init(const HAL_RTC_t *self)
{
    HAL_RCC_Cmd(self->RTC_RCC_Cmd,ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    /* Set RTC clock, if rtc sync failed means rtc not running,
    maybe clock source not running, or RTC is already 
    configured to use another source, we do a backup domain
    reset to clear RTC setting and try again. */
        
    RCC_RTCCLKConfig(self->RTC_ClockSource);
    RCC_RTCCLKCmd(ENABLE);
    
    if(RTC_WaitForSynchro()==0)
    {
        DBG_WARNING("RTC Synchro failed, reset RTC\n");

        RCC_BackupResetCmd(ENABLE);
        delay(1);
        RCC_BackupResetCmd(DISABLE);

        RCC_RTCCLKConfig(self->RTC_ClockSource);
        RCC_RTCCLKCmd(ENABLE);

        if(RTC_WaitForSynchro()==0)        
            DBG_ERROR("RTC Init failed.\n");
    }

    RTC_Init(self->RTC_InitCfg);

    PWR_BackupAccessCmd(DISABLE);
}

time_t HAL_RTC_GetTime(const HAL_RTC_t *self)
{
    struct tm ptm;
    HAL_RTC_GetTime_tm(self,&ptm);
    return mktime(&ptm);
}

void HAL_RTC_GetTime_tm(const HAL_RTC_t *self,struct tm *ptm)
{
    //Init to 0 is required
    RTC_DateTypeDef date={0};
    RTC_TimeTypeDef time={0};

    RTC_GetTime(RTC_Format_BIN, &time );
    RTC_GetDate(RTC_Format_BIN, &date );

    ptm->tm_mday = date.RTC_Date;
    ptm->tm_mon  = date.RTC_Month-1;
    ptm->tm_year = date.RTC_Year+self->RTC_Year_Offset;
    ptm->tm_hour = time.RTC_Hours+self->RTC_Hour_Offset;
    ptm->tm_min  = time.RTC_Minutes+self->RTC_Minute_Offset;
    ptm->tm_sec  = time.RTC_Seconds;
}

void HAL_RTC_SetTime_tm(const HAL_RTC_t *self,const struct tm *ptm)
{
    //Init to 0 is required
    RTC_DateTypeDef date={0};
    RTC_TimeTypeDef time={0};
    
    date.RTC_Date = ptm->tm_mday;
    date.RTC_Month = ptm->tm_mon + 1;
    date.RTC_Year = ptm->tm_year - self->RTC_Year_Offset; 
    date.RTC_WeekDay = self->RTC_WeekDay;  
    time.RTC_Hours = ptm->tm_hour - self->RTC_Hour_Offset;
    time.RTC_Minutes = ptm->tm_min - self->RTC_Minute_Offset;
    time.RTC_Seconds = ptm->tm_sec;

    PWR_BackupAccessCmd(ENABLE);
    RTC_SetTime(RTC_Format_BIN, &time);
    RTC_SetDate(RTC_Format_BIN, &date);
    PWR_BackupAccessCmd(DISABLE);
    DBG_INFO("RTC time updated\n");
    HAL_RTC_PrintTime(self);
}

void HAL_RTC_PrintTime(const HAL_RTC_t *self)
{
    struct tm ptm;
    char buffer[30];
    HAL_RTC_GetTime_tm(self,&ptm);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ptm);
    DBG_INFO("NowTime: %s\n", buffer);
}
