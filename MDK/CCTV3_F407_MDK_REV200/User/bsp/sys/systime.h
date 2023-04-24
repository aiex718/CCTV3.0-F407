#ifndef SYSTIME_H
#define SYSTIME_H

#include "bsp/platform/platform_defs.h"
typedef uint32_t SysTime_t;

SysTime_t SysTime_Get(void);
void SysTime_Inc(void);

#define delay(ms) do{                   \
    SysTime_t dst = SysTime_Get()+ms;   \
    while(SysTime_Get()<dst);           \
}while(0)

#endif
