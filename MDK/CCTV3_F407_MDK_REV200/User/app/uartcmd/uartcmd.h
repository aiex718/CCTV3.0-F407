#ifndef UARTCMD_H
#define UARTCMD_H

#include "bsp/platform/platform_defs.h"

typedef struct UartCmd_s
{
    uint8_t UartCmd_RxBuf[DEBUG_SERIAL_RX_BUFFER_SIZE];
} UartCmd_t;

void UartCmd_Service(UartCmd_t *self);

#endif
