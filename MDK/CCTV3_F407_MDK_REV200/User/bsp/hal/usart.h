#ifndef USART_H
#define USART_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/buffer.h"
#include "bsp/sys/callback.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/bitflag.h"

#include "bsp/hal/gpio.h"
#include "bsp/hal/rcc.h"
#include "bsp/hal/dma.h"


typedef enum 
{
    HAL_USART_OK =0         ,
    HAL_USART_ERROR         ,
    HAL_USART_BUSY          ,
    HAL_USART_BAD_ARGS      ,
    HAL_USART_BUF_ERROR     ,
    HAL_USART_BUF_EMPTY     ,
    HAL_USART_BUF_NOT_EMPTY ,
    HAL_USART_TOO_MANY      ,
    HAL_USART_CONFLICT      ,
    HAL_USART_DISABLED      ,
}HAL_USART_Status_t;

typedef enum 
{
    //IRQ callbacks, always invoked in ISR
    USART_CALLBACK_IRQ = 0          ,
    USART_CALLBACK_IRQ_RX_FULL      ,
    USART_CALLBACK_IRQ_TX_EMPTY     ,
    //Normal callbacks, invoked in ISR or delay to service call
    USART_CALLBACK_RX_THRSHOLD      ,
    USART_CALLBACK_RX_TIMEOUT       ,
    __NOT_CALLBACK_USART_MAX        ,
}HAL_USART_CallbackIdx_t;

__BSP_STRUCT_ALIGN typedef struct HAL_USART_s
{
    //usart
    USART_TypeDef* USARTx;
    //rcc
    HAL_RCC_Cmd_t* USART_RCC_Cmd;
    USART_InitTypeDef* USART_InitCfg;
    //nvic
    NVIC_InitTypeDef* USART_NVIC_InitCfg;
    uint16_t *USART_Enable_ITs;
    //gpio
    HAL_GPIO_pin_t* USART_TxPin;
    HAL_GPIO_pin_t* USART_RxPin;
    //dma cfg
    HAL_DMA_t* USART_TxDma_Cfg;
    HAL_DMA_t* USART_RxDma_Cfg;
    //buffers
    Buffer_uint8_t *USART_Tx_Buf,*USART_Rx_Buf;
    //rx callback settings
    uint16_t USART_Rx_Threshold;
    //rx software timeout, set to 0 to disable 
    //if enabled, it's required to call HAL_USART_Service() 
    //periodically in main thread/task
    uint16_t USART_Rx_Timeout;
    //callbacks
    Callback_t* USART_Callbacks[__NOT_CALLBACK_USART_MAX];
    //customize data structure
    void* pExtension;

    //private flags, dont use
    SysTime_t _last_rx_time;
    BitFlag8_t _callback_pending_flag;
}HAL_USART_t;

//Macro as functions
#define HAL_USART_IsEnabled(usart) ((usart)->USARTx->CR1 & USART_CR1_UE)

#define HAL_USART_IsTxEnabled(usart) ((usart)->USARTx->CR1 & USART_Mode_Tx)
#define HAL_USART_IsTxDmaEnabled(usart) ((usart)->USARTx->CR3 & USART_CR3_DMAT)
#define HAL_USART_IsTxStreamEnabled(usart) ((usart)->USARTx->CR1 & USART_CR1_TXEIE)
#define HAL_USART_IsTransmitting(usart) (                                   \
     HAL_USART_IsEnabled(usart) && HAL_USART_IsTxEnabled(usart) &&          \
    (HAL_USART_IsTxDmaEnabled(usart) || HAL_USART_IsTxStreamEnabled(usart)) )

#define HAL_USART_IsRxEnabled(usart) ((usart)->USARTx->CR1 & USART_Mode_Rx)
#define HAL_USART_IsRxDmaEnabled(usart) ((usart)->USARTx->CR3 & USART_CR3_DMAR)
#define HAL_USART_IsRxStreamEnabled(usart) ((usart)->USARTx->CR1 & USART_CR1_RXNEIE)
#define HAL_USART_IsReceiving(usart) (                                      \
     HAL_USART_IsEnabled(usart) && HAL_USART_IsRxEnabled(usart) &&          \
    (HAL_USART_IsRxDmaEnabled(usart) || HAL_USART_IsRxStreamEnabled(usart)) )


void HAL_USART_Init(HAL_USART_t* usart);
void HAL_USART_Cmd(HAL_USART_t* usart, bool en);
void HAL_USART_SetCallback(HAL_USART_t* usart, HAL_USART_CallbackIdx_t cb_idx, Callback_t* callback); 
bool HAL_USART_WriteByte_Polling(const HAL_USART_t* usart, uint8_t data);
bool HAL_USART_WriteByte(const HAL_USART_t* usart, uint8_t data);
uint16_t HAL_USART_Write(const HAL_USART_t* usart, uint8_t* data, uint16_t len);
bool HAL_USART_ReadByte_Polling(const HAL_USART_t* usart, uint8_t* data);
bool HAL_USART_ReadByte(const HAL_USART_t* usart, uint8_t* data);
uint16_t HAL_USART_Read(const HAL_USART_t* usart, uint8_t* data, uint16_t len);
HAL_USART_Status_t HAL_USART_RxStreamCmd(const HAL_USART_t* usart, bool en);
HAL_USART_Status_t HAL_USART_TxStreamCmd(const HAL_USART_t* usart, bool en);
HAL_USART_Status_t HAL_USART_SwapTxBuffer(HAL_USART_t* usart, Buffer_uint8_t* buf, Buffer_uint8_t* swap_out);
HAL_USART_Status_t HAL_USART_SwapRxBuffer(HAL_USART_t* usart, Buffer_uint8_t* buf, Buffer_uint8_t* swap_out);
HAL_USART_Status_t HAL_USART_DmaWrite(const HAL_USART_t* usart);
HAL_USART_Status_t HAL_USART_DmaRead(const HAL_USART_t* usart, uint16_t len);
HAL_USART_Status_t HAL_USART_TxStreamWake(const HAL_USART_t* usart);
HAL_USART_Status_t HAL_USART_TxDmaWake(const HAL_USART_t* usart);
void HAL_USART_IRQHandler(HAL_USART_t* usart);
void HAL_USART_Service(HAL_USART_t* usart);


#endif
