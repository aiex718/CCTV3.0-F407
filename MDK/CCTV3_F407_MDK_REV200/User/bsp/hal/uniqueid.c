#include "bsp/hal/uniqueid.h"


void HAL_UniqueID_Init(const HAL_UniqueID_t *self)
{
    ;//nothing
}

uint32_t HAL_UniqueID_Read(const HAL_UniqueID_t *self,uint8_t offset)
{
    if(offset>=self->UniqueID_Len)
        return 0;
    else
        return *((uint32_t *)self->UniqueID_Addrs[offset]);
}

uint8_t HAL_UniqueID_GetLen(const HAL_UniqueID_t *self)
{
    return self->UniqueID_Len;
}
