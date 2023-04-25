#ifndef RNG_H
#define RNG_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"

typedef struct
{
    HAL_RCC_Cmd_t *Rng_RCC_Cmd;
}HAL_Rng_t;

void HAL_Rng_Init(HAL_Rng_t* self);
uint32_t HAL_Rng_Gen(HAL_Rng_t* self);
uint32_t HAL_Rng_Gen_Range(HAL_Rng_t* self,uint32_t min,uint32_t max);
uint32_t HAL_Rng_Gen_Smaller(HAL_Rng_t* self,uint32_t limit);
uint32_t HAL_Rng_Gen_Larger(HAL_Rng_t* self,uint32_t limit);


#endif
