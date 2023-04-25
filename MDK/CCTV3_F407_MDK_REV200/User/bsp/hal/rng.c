#include "bsp/hal/rng.h"

void HAL_Rng_Init(HAL_Rng_t* self)
{
    HAL_RCC_Cmd(self->Rng_RCC_Cmd,ENABLE);
    RNG_Cmd(ENABLE);
}

uint32_t HAL_Rng_Gen(HAL_Rng_t* self)
{
    while(RNG_GetFlagStatus(RNG_FLAG_DRDY)==RESET);
    return RNG_GetRandomNumber();
}

uint32_t HAL_Rng_Gen_Range(HAL_Rng_t* self,uint32_t min,uint32_t max)
{
    uint32_t range = max - min;
    uint32_t rand = HAL_Rng_Gen(self);
    return (rand % range) + min;
}

uint32_t HAL_Rng_Gen_Smaller(HAL_Rng_t* self,uint32_t limit)
{
    uint32_t rand = HAL_Rng_Gen(self);
    return rand % limit;
}

uint32_t HAL_Rng_Gen_Larger(HAL_Rng_t* self,uint32_t limit)
{
    uint32_t range = 0xffffffff - limit;
    uint32_t rand = HAL_Rng_Gen_Smaller(self,range);
    return limit + range + 1;
}
