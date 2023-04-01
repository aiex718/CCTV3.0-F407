#include "mco.h"

void HAL_MCO_Init(HAL_MCO_t* self)
{
    HAL_GPIO_InitPin(self->MCO_Pin);
    if(self->MCO_Idx == MCO1)
        RCC_MCO1Config(self->MCO_Source, self->MCO_ClkDiv);
    else if(self->MCO_Idx == MCO2)
        RCC_MCO2Config(self->MCO_Source, self->MCO_ClkDiv);
}
