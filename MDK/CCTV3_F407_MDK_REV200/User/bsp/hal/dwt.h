#ifndef __DWT_H__
#define __DWT_H__

#include "bsp/platform/platform_defs.h"

void DWT_Init(void);
uint32_t DWT_GetCycle(void);
void DWT_Clear(void);

#endif
