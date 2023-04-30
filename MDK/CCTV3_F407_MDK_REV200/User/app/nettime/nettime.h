#ifndef NETTIME_H
#define NETTIME_H

#include "bsp/platform/platform_defs.h"

typedef struct NetTime_s{
    const char* NTP_Server;
} NetTime_t;

void NetTime_Init(NetTime_t *self);
void NetTime_Sntp_Poll_Callback(uint32_t unix_epoch_sec);

#endif
