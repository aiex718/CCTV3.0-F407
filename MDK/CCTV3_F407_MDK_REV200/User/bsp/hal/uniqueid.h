#ifndef UNIQUEID_H
#define UNIQUEID_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"

typedef struct HAL_UniqueID_s
{
    uint32_t* UniqueID_Addrs;
    uint8_t UniqueID_Len;
}HAL_UniqueID_t;

void HAL_UniqueID_Init(const HAL_UniqueID_t *self);
uint32_t HAL_UniqueID_Read(const HAL_UniqueID_t *self,uint8_t offset);
uint8_t HAL_UniqueID_GetLen(const HAL_UniqueID_t *self);

#endif
