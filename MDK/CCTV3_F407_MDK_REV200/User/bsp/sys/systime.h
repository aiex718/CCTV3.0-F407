#ifndef SYSTIME_H
#define SYSTIME_H

#include "bsp/platform/platform_defs.h"
typedef __IO uint32_t Systime_t;

Systime_t Systime_Get(void);
void Systime_Inc(void);

#define delay(ms) do{                   \
    Systime_t dst = Systime_Get()+ms;   \
    while(Systime_Get()<dst);           \
}while(0)

#endif
