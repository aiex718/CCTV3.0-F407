#include "bsp/hal/usart.h"
#include "bsp/sys/systime.h"

// private functions declaration
static HAL_USART_Status_t HAL_USART_TxStreamCmd(const HAL_USART_t* usart, bool en);

void HAL_USART_Init(const HAL_USART_t* usart)
{
    HAL_RCC_Cmd(usart->USART_RCC_Cmd,true);
    USART_Init(usart->USARTx,(USART_InitTypeDef*)usart->USART_InitCfg);

    if(usart->USART_NVIC_InitCfg)
    {
        uint16_t *it_ptr = (uint16_t *)usart->USART_Enable_ITs;
        NVIC_Init((NVIC_InitTypeDef*)usart->USART_NVIC_InitCfg);
        while (*it_ptr)
            USART_ITConfig(usart->USARTx, *it_ptr++, ENABLE);
    }

    if(usart->USART_Callback_PendingQueue)
        Buffer_Clear(usart->USART_Callback_PendingQueue);

    if(usart->USART_InitCfg->USART_Mode | USART_Mode_Tx)
    {
        HAL_GPIO_InitPin(usart->USART_TxPin);
        if(usart->USART_Tx_Buf)
            Buffer_Clear(usart->USART_Tx_Buf);
    }
    
    if(usart->USART_InitCfg->USART_Mode | USART_Mode_Rx)
    {
	    HAL_GPIO_InitPin(usart->USART_RxPin);
        if(usart->USART_Rx_Buf)
            Buffer_Clear(usart->USART_Rx_Buf);
    }
}

bool HAL_USART_WriteByte(const HAL_USART_t* usart, uint8_t data)
{
    if(HAL_USART_IsTxDmaEnabled(usart)) 
        return false;
    else 
    {
        bool ret = Buffer_Queue_Push_uint8_t(usart->USART_Tx_Buf, data);
        if(HAL_USART_IsTransmitting(usart)==false) 
            HAL_USART_TxStreamCmd(usart,true);
        return ret;
    }
}

uint16_t HAL_USART_Write(const HAL_USART_t* usart, uint8_t* data, uint16_t len)
{
    if(HAL_USART_IsTxDmaEnabled(usart)) 
        return 0;
    else if(len)
    {
        uint16_t ret = Buffer_Queue_PushArray_uint8_t(usart->USART_Tx_Buf, data, len);
        if(HAL_USART_IsTransmitting(usart)==false) 
            HAL_USART_TxStreamCmd(usart,true);
        return ret;
    }
    return 0;
}

bool HAL_USART_ReadByte(const HAL_USART_t* usart, uint8_t* data)
{
    if(HAL_USART_IsRxDmaEnabled(usart)) 
        return false;
    else
        return Buffer_Queue_Pop_uint8_t(usart->USART_Rx_Buf, data);
}

uint16_t HAL_USART_Read(const HAL_USART_t* usart, uint8_t* data, uint16_t len)
{
    if(HAL_USART_IsRxDmaEnabled(usart)) 
        return 0;
    else
        return Buffer_Queue_PopArray_uint8_t(usart->USART_Rx_Buf, data, len);
}

HAL_USART_Status_t HAL_USART_TxStreamCmd(const HAL_USART_t* usart, bool en)
{
    USART_TypeDef *usart_hw = usart->USARTx;
    if(HAL_USART_IsTxDmaEnabled(usart)) 
        return HAL_USART_CONFLICT;
    else if (en)
    {
        if(HAL_USART_IsTransmitting(usart)) 
            return HAL_USART_BUSY;
        else if(usart->USART_Tx_Buf == NULL)
            return HAL_USART_BUF_ERROR;

        USART_ClearFlag(usart_hw, USART_FLAG_TC);
        USART_ITConfig(usart_hw, USART_IT_TC, DISABLE);
        USART_ITConfig(usart_hw, USART_IT_TXE, ENABLE);
    }
    else
    {
        USART_ITConfig(usart_hw, USART_IT_TXE, DISABLE);
    }
    return HAL_USART_OK;
}

HAL_USART_Status_t HAL_USART_RxStreamCmd(const HAL_USART_t* usart, bool en)
{
    USART_TypeDef *usart_hw = usart->USARTx;
    if(HAL_USART_IsRxDmaEnabled(usart)) 
        return HAL_USART_CONFLICT;
    else if (en)
    {
        if(HAL_USART_IsReceiving(usart)) 
            return HAL_USART_BUSY;
        else if(usart->USART_Rx_Buf == NULL)
            return HAL_USART_BUF_ERROR;
        USART_ClearFlag(usart_hw, USART_FLAG_RXNE);
        USART_ITConfig(usart_hw, USART_IT_RXNE, ENABLE);
    }
    else
    {
        USART_ITConfig(usart_hw, USART_IT_RXNE, DISABLE);
        USART_ClearFlag(usart_hw, USART_FLAG_RXNE);
    }
    return HAL_USART_OK;
}

HAL_USART_Status_t HAL_USART_SwapTxBuffer(HAL_USART_t* usart,
    Buffer_uint8_t* buf, Buffer_uint8_t* swap_out)
{
    BSP_UNUSED_ARG(swap_out);
    if(HAL_USART_IsTransmitting(usart)) 
        return HAL_USART_BUSY;
    swap_out = usart->USART_Tx_Buf;
    usart->USART_Tx_Buf = buf;
    return HAL_USART_OK;
}

HAL_USART_Status_t HAL_USART_SwapRxBuffer(HAL_USART_t* usart,
    Buffer_uint8_t* buf, Buffer_uint8_t* swap_out)
{
    BSP_UNUSED_ARG(swap_out);
    if(HAL_USART_IsReceiving(usart)) 
        return HAL_USART_BUSY;
    swap_out = usart->USART_Rx_Buf;
    usart->USART_Rx_Buf = buf;
    return HAL_USART_OK;
}

HAL_USART_Status_t HAL_USART_DmaWrite(const HAL_USART_t* usart)
{
    Buffer_uint8_t* queue = usart->USART_Tx_Buf;
    const HAL_DMA_t *dma_cfg = usart->USART_TxDma_Cfg;
    USART_TypeDef* hw_USART = usart->USARTx;

    if(HAL_USART_IsTransmitting(usart)) 
        return HAL_USART_BUSY;
    else if(dma_cfg==NULL || queue==NULL)
        return HAL_USART_BAD_ARGS;
    else if (Buffer_Queue_IsEmpty(queue))
        return HAL_USART_BUF_EMPTY;
    else if (queue->buf_ptr!=queue->r_ptr)
        return HAL_USART_BUF_ERROR;
    else
    {
        //config DMA
        HAL_DMA_Init(dma_cfg);
        HAL_DMA_SetMemAddr(dma_cfg, queue->buf_ptr);
        HAL_DMA_SetPeriphAddr(dma_cfg , hw_USART->DR);
        HAL_DMA_SetNumOfData(dma_cfg, Buffer_Queue_GetSize(queue));
        HAL_DMA_Cmd(dma_cfg,true);

        //config IT, clear TC for robustness
        USART_ClearFlag(hw_USART, USART_FLAG_TC);
        USART_ITConfig(hw_USART, USART_IT_TC, ENABLE);
        //TXE is only used for stream mode
        USART_ITConfig(hw_USART, USART_IT_TXE, DISABLE);

        USART_DMACmd(hw_USART, USART_DMAReq_Tx, ENABLE);
    }

    return HAL_USART_OK;
}

HAL_USART_Status_t HAL_USART_DmaRead(const HAL_USART_t* usart,uint16_t len)
{
    Buffer_uint8_t* queue = usart->USART_Rx_Buf;
    const HAL_DMA_t *dma_cfg = usart->USART_RxDma_Cfg;
    
    if(HAL_USART_IsReceiving(usart)) 
        return HAL_USART_BUSY;
    else if(dma_cfg==NULL || queue==NULL)
        return HAL_USART_BAD_ARGS;
    else if (Buffer_Queue_IsEmpty(queue)==false)
        return HAL_USART_BUF_NOT_EMPTY;
    else if (len > Buffer_Queue_GetMaxCapacity(queue))
        return HAL_USART_TOO_MANY;
    else
    {
        Buffer_Clear(queue);
        queue->w_ptr = queue->buf_ptr+len;
        //config DMA
        HAL_DMA_Init(dma_cfg);
        HAL_DMA_SetMemAddr(dma_cfg , queue->buf_ptr);
        HAL_DMA_SetPeriphAddr(dma_cfg ,usart->USARTx->DR);
        HAL_DMA_SetNumOfData(dma_cfg ,len);
        HAL_DMA_Cmd(dma_cfg,true);

        //config IT,
        USART_ITConfig(usart->USARTx, USART_IT_RXNE, DISABLE);

        USART_DMACmd(usart->USARTx, USART_DMAReq_Rx, ENABLE);
    }
    return HAL_USART_OK;
}

void HAL_USART_IRQHandler(HAL_USART_t* usart)
{
    Callback_Invoke(usart,usart->USART_IT_Callback);

    if(USART_GetITStatus(usart->USARTx, USART_IT_TXE) != RESET)
    {
        uint8_t data;
        if(Buffer_Queue_Pop_uint8_t(usart->USART_Tx_Buf, &data))
            USART_SendData(usart->USARTx, data);
        else
        {
            USART_ITConfig(usart->USARTx, USART_IT_TXE, DISABLE);
            Callback_InvokeOrQueue_IRq(usart,
                usart->USART_Tx_Empty_Callback ,
                usart->USART_Callback_PendingQueue);
        }
    }
    if(USART_GetITStatus(usart->USARTx, USART_IT_TC) != RESET)
    {
        USART_ITConfig(usart->USARTx, USART_IT_TC, DISABLE);
        if(usart->USART_TxDma_Cfg)
        {
            USART_DMACmd(usart->USARTx, USART_DMAReq_Tx, DISABLE);
            HAL_DMA_Cmd(usart->USART_TxDma_Cfg,false);
            Buffer_Clear(usart->USART_Tx_Buf);
        }
    }
    if(USART_GetITStatus(usart->USARTx, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(usart->USARTx);
        if(Buffer_Queue_IsFull(usart->USART_Rx_Buf))
            Callback_Invoke(usart,usart->USART_Rx_Full_Callback);
        
        if(Buffer_Queue_Push_uint8_t(usart->USART_Rx_Buf, data))
            usart->_last_rx_time = Systime_Get();
        else
            Callback_Invoke(usart,usart->USART_Rx_Dropped_Callback);

        if (Buffer_Queue_GetSize(usart->USART_Rx_Buf) >= 
                usart->USART_Rx_Threshold)
        {
            Callback_InvokeOrQueue_IRq(usart,
                usart->USART_Rx_ThrsReach_Callback,
                usart->USART_Callback_PendingQueue);
        }
    }
    if(USART_GetITStatus(usart->USARTx, USART_IT_IDLE) != RESET)
    {
        USART_ReceiveData(usart->USARTx); //clear IDLE flag
        if(usart->USART_RxDma_Cfg)
        {
            uint16_t ndt;
            USART_DMACmd(usart->USARTx, USART_DMAReq_Rx, DISABLE);
            HAL_DMA_Cmd(usart->USART_RxDma_Cfg,false);
            ndt = HAL_DMA_GetNumOfData(usart->USART_RxDma_Cfg);
            usart->USART_Rx_Buf->w_ptr -= ndt;
        }
        Callback_InvokeOrQueue_IRq(usart,
                usart->USART_Rx_Timeout_Callback,
                usart->USART_Callback_PendingQueue);
    }
}

void HAL_USART_Service(HAL_USART_t* usart)
{
    Callback_t* cb;
    //check rx timeout
    if( usart->USART_Rx_Timeout && Buffer_Queue_IsEmpty(usart->USART_Rx_Buf)==false &&
        (Systime_Get() - usart->_last_rx_time) >= usart->USART_Rx_Timeout)
    {
        Callback_Invoke(usart,usart->USART_Rx_Timeout_Callback);
    }
    //Execute pending callbacks
    while(Buffer_Queue_Pop_CallbackP_t(usart->USART_Callback_PendingQueue, &cb))
        Callback_Invoke(usart,cb);
}
