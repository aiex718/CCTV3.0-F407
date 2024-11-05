#ifndef __PERFMON_H__
#define __PERFMON_H__

#include "bsp/platform/platform_defs.h"

void PerfMon_Init(void);
void PerfMon_Serivce(void);
uint32_t PerfMon_GetLoopFreq(void);

#endif
