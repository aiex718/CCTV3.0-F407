#ifndef RNG_H
#define RNG_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"

typedef struct HAL_Rng_s
{
    HAL_RCC_Cmd_t *Rng_RCC_Cmd;
}HAL_Rng_t;

void HAL_Rng_Init(const HAL_Rng_t* self);
uint32_t HAL_Rng_Gen(const HAL_Rng_t* self);
uint32_t HAL_Rng_Gen_Range(const HAL_Rng_t* self,uint32_t min,uint32_t max);
uint32_t HAL_Rng_Gen_Smaller(const HAL_Rng_t* self,uint32_t limit);
uint32_t HAL_Rng_Gen_Larger(const HAL_Rng_t* self,uint32_t limit);


#endif
