#ifndef SYSTIMER_H
#define SYSTIMER_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/systime.h"

#ifndef SYSTIMER_PERIOD_T
    #define SYSTIMER_PERIOD_T uint16_t
#endif

__BSP_STRUCT_ALIGN typedef struct SysTimer_s{
    SYSTIMER_PERIOD_T period;
    SysTime_t last;
} SysTimer_t;

#define SysTimer_IsElapsed(systmr) ((SysTime_Get() - (systmr)->last) >= (systmr)->period)
#define SysTimer_Reset(systmr) ((systmr)->last = SysTime_Get())

__STATIC_INLINE void SysTimer_Init(SysTimer_t* systmr, uint16_t period)
{
    systmr->period = period;
    systmr->last = SysTime_Get();
}


#endif
