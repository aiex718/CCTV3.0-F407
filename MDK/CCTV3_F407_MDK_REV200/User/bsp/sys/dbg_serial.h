#ifndef DBG_SERIAL_H
#define DBG_SERIAL_H

#include "bsp/platform/platform_defs.h"
#include "bsp/platform/platform_opts.h"
#include "bsp/hal/usart.h"
#include "bsp/sys/concurrent_queue.h"
#include "bsp/sys/buffer.h"
#include "bsp/sys/callback.h"

#ifndef DEBUG_SERIAL_USART_TX_BUFFER_SIZE
    #define DEBUG_SERIAL_USART_TX_BUFFER_SIZE 64
#endif
#ifndef DEBUG_SERIAL_USART_RX_BUFFER_SIZE
    #define DEBUG_SERIAL_USART_RX_BUFFER_SIZE 32
#endif
#ifndef DEBUG_SERIAL_TX_BUFFER_SIZE
    #define DEBUG_SERIAL_TX_BUFFER_SIZE 256
#endif
#ifndef DEBUG_SERIAL_RX_BUFFER_SIZE
    #define DEBUG_SERIAL_RX_BUFFER_SIZE 32
#endif

#ifndef DBG_SERIAL_ENABLE_DMA
    #define DBG_SERIAL_ENABLE_DMA 0
#endif

#ifndef DBG_SERIAL_USING_USART_ISR
    #define DBG_SERIAL_USING_USART_ISR 1
#endif

#ifndef DBG_SERIAL_SHOW_INFO
    #define DBG_SERIAL_SHOW_INFO  0
#endif

#ifndef DBG_SERIAL_SHOW_WARNING
    #define DBG_SERIAL_SHOW_WARNING 1
#endif

#ifndef DBG_SERIAL_SHOW_ERROR
    #define DBG_SERIAL_SHOW_ERROR  1
#endif

typedef struct DBG_Serial_s
{
    Concurrent_Queue_uint8_t *tx_con_queue,*rx_con_queue;
    HAL_USART_t *hal_usart;
    bool safe_mode;

    //callbacks attatch to usart, dont modify
    Callback_t _tx_empty_cb, _rx_timeout_cb;
}DBG_Serial_t;

void DBG_Serial_Init(DBG_Serial_t *self);
void DBG_Serial_AttachUSART(DBG_Serial_t *self,HAL_USART_t *hal_usart);
void DBG_Serial_Cmd(DBG_Serial_t *self,bool en);
void DBG_Serial_SafeMode(DBG_Serial_t *self,bool en);
void DBG_Serial_Service(DBG_Serial_t *self);
uint16_t DBG_Serial_ReadLine(DBG_Serial_t *self,uint8_t* buf, uint16_t buf_len);

#define DBG_PRINTF(x,lv,...)   printf(lv "%s#%d:" x,__MODULE__,__LINE__,##__VA_ARGS__)

#if DBG_SERIAL_SHOW_INFO
#define DBG_INFO(x,...) DBG_PRINTF(x,"",##__VA_ARGS__)
#else
#define DBG_INFO(x,...)
#endif

#if DBG_SERIAL_SHOW_WARNING
#define DBG_WARNING(x,...)  DBG_PRINTF(x,"WARNING,",##__VA_ARGS__)
#else
#define DBG_WARNING(x,...)
#endif

#if DBG_SERIAL_SHOW_ERROR
#define DBG_ERROR(x,...)    DBG_PRINTF(x,"ERROR,",##__VA_ARGS__)
#else
#define DBG_ERROR(x,...)
#endif

//include for instance
#include "bsp/platform/periph/peri_dbgserial.h" 

#endif
