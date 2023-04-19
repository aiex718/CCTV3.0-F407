#include "systime.h"

static __IO SysTime_t systime_tick;

SysTime_t SysTime_Get(void)
{
    return systime_tick;
}

void SysTime_Inc(void)
{
    ++systime_tick;
}
