#include "app/nettime/nettime.h"

#include "lwip/apps/sntp.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/platform/periph/peri_rtc.h"
#include "bsp/platform/device/dev_config_storage.h"

//This module enable lwip sntp client, and set RTC time from sntp server
void NetTime_Init(NetTime_t *self)
{   
    if(self->NetTime_Enable)
    {
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_init();
        sntp_setservername(0,self->NetTime_SNTP_Server);
        DBG_INFO("NetTime inited, sntp server:%s\n",self->NetTime_SNTP_Server);
    }
}

void NetTime_ConfigSet(NetTime_t *self,const NetTime_ConfigFile_t *config)
{
    self->NetTime_Enable        = config->NetTime_Enable;
    self->NetTime_SNTP_Server   = config->NetTime_SNTP_Server;
}

void NetTime_ConfigExport(const NetTime_t *self,NetTime_ConfigFile_t *config)
{
    config->NetTime_Enable = self->NetTime_Enable;

    BSP_STRNCPY(config->NetTime_SNTP_Server,self->NetTime_SNTP_Server,sizeof(config->NetTime_SNTP_Server));
    config->NetTime_SNTP_Server[sizeof(config->NetTime_SNTP_Server)-1] = '\0';
}


bool NetTime_IsConfigValid(NetTime_t *self,const NetTime_ConfigFile_t *config)
{
    return config != NULL &&
    (config->NetTime_Enable== true || config->NetTime_Enable== false) && 
    config->NetTime_SNTP_Server != NULL && (int)(config->NetTime_SNTP_Server) != -1 &&
    strlen(config->NetTime_SNTP_Server)>0;
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
