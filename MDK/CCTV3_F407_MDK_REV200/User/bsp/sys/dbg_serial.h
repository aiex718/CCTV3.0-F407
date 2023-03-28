#ifndef DBG_SERIAL_H
#define DBG_SERIAL_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/usart.h"
#include "bsp/sys/concurrent_queue.h"
#include "bsp/sys/buffer.h"
#include "bsp/sys/callback.h"

#ifndef DBG_SERIAL_ENABLE_DMA
    #define DBG_SERIAL_ENABLE_DMA 1
#endif

typedef struct 
{
    Concurrent_Queue_uint8_t *tx_con_queue;
    Buffer_uint8_t *rx_buf;
    HAL_USART_t *usart;

    //callbacks attatch to usart, dont modify
    Callback_t *_tx_empty_cb, *_rx_timeout_cb;
}DBG_Serial_t;

extern DBG_Serial_t* DBG_Serial;

void DBG_Serial_Init(DBG_Serial_t *self);
void DBG_Serial_AttachUSART(DBG_Serial_t *self,HAL_USART_t *usart);
void DBG_Serial_Service(DBG_Serial_t *self);
uint16_t DBG_Serial_ReadLine(DBG_Serial_t *self,uint8_t* buf, uint16_t buf_len);


#define DBG_LF(x) ("%s#%d:" x,__MODULE__,__LINE__)
#define DBG_ARG(x,...) ("%s#%d:" x,__MODULE__,__LINE__,__VA_ARGS__)


#endif
