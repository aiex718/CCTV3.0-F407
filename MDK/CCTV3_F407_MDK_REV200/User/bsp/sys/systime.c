#include "systime.h"

static Systime_t systime_tick;

Systime_t Systime_Get(void)
{
    return systime_tick;
}

void Systime_Inc(void)
{
    ++systime_tick;
}
