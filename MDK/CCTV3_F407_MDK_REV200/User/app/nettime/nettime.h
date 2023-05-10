#ifndef NETTIME_H
#define NETTIME_H

#include "bsp/platform/platform_defs.h"


typedef struct NetTime_ConfigFile_s
{
    uint8_t NetTime_Enable;
    uint8_t __padding[3];
    char NetTime_SNTP_Server[64];
}NetTime_ConfigFile_t;


typedef struct NetTime_s{
    const char* NetTime_SNTP_Server;
    bool NetTime_Enable;
}NetTime_t;

void NetTime_Init(NetTime_t *self);
void NetTime_ConfigSet(NetTime_t *self,const NetTime_ConfigFile_t *config);
void NetTime_ConfigExport(const NetTime_t *self,NetTime_ConfigFile_t *config);
bool NetTime_IsConfigValid(NetTime_t *self,const NetTime_ConfigFile_t *config);

void NetTime_Sntp_Poll_Callback(uint32_t unix_epoch_sec);

#endif
