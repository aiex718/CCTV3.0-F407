#ifndef USART_H
#define USART_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/array.h"
#include "bsp/sys/buffer.h"
#include "bsp/sys/callback.h"

#include "bsp/hal/gpio.h"
#include "bsp/hal/rcc.h"
#include "bsp/hal/dma.h"
#include "bsp/sys/systime.h"

typedef enum 
{
    HAL_USART_OK                    =0,
    HAL_USART_ERROR                 =1,
    HAL_USART_BUSY                  =2,
    HAL_USART_BAD_ARGS              =4,
    HAL_USART_BUF_ERROR             =8,
    HAL_USART_BUF_EMPTY             =16,
    HAL_USART_BUF_NOT_EMPTY         =32,
    HAL_USART_TOO_MANY              =64,
    HAL_USART_CONFLICT              =128,
}HAL_USART_Status_t;

__BSP_STRUCT_ALIGN typedef struct
{
    //usart
    USART_TypeDef* USARTx;
    //rcc
    HAL_RCC_Cmd_t* USART_RCC_Cmd;
    //gpio
    HAL_GPIO_pin_t* USART_TxPin;
    HAL_GPIO_pin_t* USART_RxPin;
    USART_InitTypeDef* USART_InitCfg;
    //nvic
    NVIC_InitTypeDef* USART_NVIC_InitCfg;
    uint16_t *USART_Enable_ITs;
    //dma cfg
    HAL_DMA_t* USART_TxDma_Cfg;
    HAL_DMA_t* USART_RxDma_Cfg;
    //buffers
    Buffer_uint8_t *USART_Tx_Buf,*USART_Rx_Buf;
    //rx callback settings
    uint16_t USART_Rx_Threshold;
    uint16_t USART_Rx_Timeout;
    //callbacks
    CallbackIRq_t* USART_IT_Callback;
    Callback_t* USART_Tx_Empty_Callback;
    Callback_t* USART_Rx_ThrsReach_Callback;
    Callback_t* USART_Rx_Timeout_Callback;
    CallbackIRq_t* USART_Rx_Dropped_Callback;
    CallbackIRq_t* USART_Rx_Full_Callback;
    Buffer_CallbackP_t *USART_Callback_PendingQueue;
    //customize data structure
    void* pExtension;

    //private flags, dont use
    Systime_t _last_rx_time;
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

#define HAL_USART_Cmd(usart,en) USART_Cmd((usart)->USARTx,(en)?ENABLE:DISABLE)

void HAL_USART_Init(const HAL_USART_t* usart);
bool HAL_USART_WriteByte(const HAL_USART_t* usart, uint8_t data);
uint16_t HAL_USART_Write(const HAL_USART_t* usart, uint8_t* data, uint16_t len);
bool HAL_USART_ReadByte(const HAL_USART_t* usart, uint8_t* data);
uint16_t HAL_USART_Read(const HAL_USART_t* usart, uint8_t* data, uint16_t len);
HAL_USART_Status_t HAL_USART_RxStreamCmd(const HAL_USART_t* usart, bool en);
HAL_USART_Status_t HAL_USART_SwapTxBuffer(HAL_USART_t* usart, Buffer_uint8_t* buf, Buffer_uint8_t* swap_out);
HAL_USART_Status_t HAL_USART_SwapRxBuffer(HAL_USART_t* usart, Buffer_uint8_t* buf, Buffer_uint8_t* swap_out);
HAL_USART_Status_t HAL_USART_DmaWrite(const HAL_USART_t* usart);
HAL_USART_Status_t HAL_USART_DmaRead(const HAL_USART_t* usart, uint16_t len);
void HAL_USART_IRQHandler(HAL_USART_t* usart);
void HAL_USART_Service(HAL_USART_t* usart);


#endif
