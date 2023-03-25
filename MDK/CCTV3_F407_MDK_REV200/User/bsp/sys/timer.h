#ifndef TIMER_H
#define TIMER_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/systime.h"

#ifndef TIMER_PERIOD_T
    #define TIMER_PERIOD_T uint16_t
#endif

__BSP_STRUCT_ALIGN typedef struct {
    TIMER_PERIOD_T period;
    Systime_t last;
} Timer_t;

#define Timer_IsElapsed(timer) ((Systime_Get() - (timer)->last) >= (timer)->period)
#define Timer_Reset(timer) ((timer)->last = Systime_Get())

__STATIC_INLINE void Timer_Init(Timer_t* timer, uint16_t period)
{
    timer->period = period;
    timer->last = Systime_Get();
}


#endif
