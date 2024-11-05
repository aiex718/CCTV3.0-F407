
#include "bsp/sys/perfmon.h"
#include "bsp/sys/systime.h"
#include "bsp/hal/dwt.h"

static uint32_t loop_cnt;
static SysTime_t last_loop_time;

void PerfMon_Init(void)
{
    last_loop_time = SysTime_Get();
    DWT_Init();
}

void PerfMon_Serivce(void)
{
    loop_cnt++;
}

uint32_t PerfMon_GetLoopFreq(void)
{
    SysTime_t now = SysTime_Get();
    uint32_t result = loop_cnt*1000/(now - last_loop_time);

    loop_cnt = 0;
    last_loop_time = now;

    return result;
}

void PerfMon_StartMeasure(void)
{
    DWT_Clear();
}

uint32_t PerfMon_StopMeasure(void)
{
    return DWT_GetCycle();
}


