#ifndef UNIQUEID_H
#define UNIQUEID_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"

typedef struct 
{
    const uint32_t* UniqueID_Addrs;
    const uint8_t UniqueID_Len;
}HAL_UniqueID_t;

void HAL_UniqueID_Init(HAL_UniqueID_t *self);
uint32_t HAL_UniqueID_Read(HAL_UniqueID_t *self,uint8_t offset);

#endif
