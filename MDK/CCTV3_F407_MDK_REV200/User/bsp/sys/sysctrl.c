#include "bsp/sys/sysctrl.h"
#include "bsp/sys/systime.h"

__IO SysTime_t ResetTime=0;

void SysCtrl_ResetAfter(uint16_t ms)
{
    ResetTime = SysTime_Get() + ms;
}

void SysCtrl_Service(void)
{
    if(ResetTime && SysTime_Get() > ResetTime)
        SysCtrl_Reset();
}

