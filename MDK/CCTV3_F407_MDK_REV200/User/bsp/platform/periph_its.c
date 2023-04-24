#include "bsp/platform/periph_list.h"

//periph interrupt handlers
void USART3_IRQHandler(void)
{
    HAL_USART_IRQHandler(DBG_Serial->hal_usart);
}

