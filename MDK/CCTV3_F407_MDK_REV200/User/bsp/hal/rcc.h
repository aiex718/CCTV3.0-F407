#ifndef RCC_H
#define RCC_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/boolean.h"

__HAL_STRUCT_ALIGN typedef struct 
{
    uint32_t RCC_AHB1Periph;
    uint32_t RCC_AHB2Periph;
    uint32_t RCC_AHB3Periph;
    uint32_t RCC_APB1Periph;
    uint32_t RCC_APB2Periph;
}HAL_RCC_Cmd_t;

void HAL_RCC_Cmd(const HAL_RCC_Cmd_t* cmd,const bool en);


#endif
