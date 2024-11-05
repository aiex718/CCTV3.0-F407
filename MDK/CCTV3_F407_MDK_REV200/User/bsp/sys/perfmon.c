
#include "bsp/sys/perfmon.h"
#include "bsp/sys/systime.h"

static uint32_t cnt;
static SysTime_t lasttime;

void PerfMon_Init(void)
{
    lasttime = SysTime_Get();
}

void PerfMon_Serivce(void)
{
    cnt++;
}

uint32_t PerfMon_GetLoopFreq(void)
{
    SysTime_t now = SysTime_Get();
    uint32_t result = cnt*1000/(now - lasttime);

    cnt = 0;
    lasttime = now;

    return result;
}
