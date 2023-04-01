#ifndef MCO_H
#define MCO_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"

typedef enum {
    MCO_NOT_SET = 0,
    MCO1,
    MCO2,
    __NOT_MCO_MAX,
}MCO_Idx_t;

__BSP_STRUCT_ALIGN typedef struct
{
    MCO_Idx_t MCO_Idx;
    HAL_GPIO_pin_t* MCO_Pin;
    uint32_t MCO_Source;
    uint32_t MCO_ClkDiv;
}HAL_MCO_t;

#endif
