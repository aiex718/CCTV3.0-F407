#include "eth/apps/nettime/nettime.h"
#include "lwip/apps/sntp.h"

#include "bsp/platform/periph/peri_rtc.h"

//This module enable lwip sntp client, and set RTC time from sntp server
void NetTime_Init(NetTime_t *self)
{
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	
	sntp_init();
	
    sntp_setservername(0,self->NTP_Server);
}

//callback from sntpd
void NetTime_Sntp_Poll_Callback(uint32_t unix_epoch_sec)
{
    struct tm _tm;
    time_t t;

    if(unix_epoch_sec==0) return ;

    //printf("ntp receive time epoch:%u\n",unix_epoch_sec);  
    
    t = unix_epoch_sec;
    localtime_r(&t, &_tm);

    HAL_RTC_SetTime_tm(Peri_RTC, &_tm);
}
