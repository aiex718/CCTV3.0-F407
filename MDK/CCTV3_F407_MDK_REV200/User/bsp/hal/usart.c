#include "bsp/hal/usart.h"
#include "bsp/sys/systime.h"

void HAL_USART_Init(HAL_USART_t* usart)
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

    if(usart->USART_InitCfg->USART_Mode | USART_Mode_Tx)
    {
        HAL_GPIO_InitPin(usart->USART_TxPin);
        if(usart->USART_Tx_Buf)
            Buffer_Clear(usart->USART_Tx_Buf);
        if(usart->USART_TxDma_Cfg)
            HAL_DMA_Init(usart->USART_TxDma_Cfg);
    }
    
    if(usart->USART_InitCfg->USART_Mode | USART_Mode_Rx)
    {
	    HAL_GPIO_InitPin(usart->USART_RxPin);
        if(usart->USART_Rx_Buf)
            Buffer_Clear(usart->USART_Rx_Buf);
        if(usart->USART_RxDma_Cfg)
            HAL_DMA_Init(usart->USART_RxDma_Cfg);
    }

    BSP_ARR_CLEAR(usart->USART_Callbacks);
    usart->pExtension = NULL;
    usart->_callback_pending_flag = 0;
    usart->_last_rx_time = 0;
}

void HAL_USART_Cmd(HAL_USART_t* usart, bool en)
{
    USART_Cmd((usart)->USARTx,(en)?ENABLE:DISABLE);

    if(en==false)
    {   
        if(HAL_USART_IsTxStreamEnabled(usart))
            HAL_USART_TxStreamCmd(usart,false);
        //this will trigger Tx_Empty callback
        if(HAL_USART_IsTxDmaEnabled(usart))
        {
            USART_DMACmd(usart->USARTx, USART_DMAReq_Tx, DISABLE);
            HAL_DMA_Cmd(usart->USART_TxDma_Cfg,false);
        }
        if(usart->USART_Tx_Buf)
            Buffer_Clear(usart->USART_Tx_Buf);

        if(HAL_USART_IsRxStreamEnabled(usart))
            HAL_USART_RxStreamCmd(usart,false);
        //this will trigger RX_TIMEOUT callback
        if(HAL_USART_IsRxDmaEnabled(usart))
        {
            USART_DMACmd(usart->USARTx, USART_DMAReq_Rx, DISABLE);
            HAL_DMA_Cmd(usart->USART_RxDma_Cfg,false);
        }
        if(usart->USART_Rx_Buf)
            Buffer_Clear(usart->USART_Rx_Buf);
    }
}

void HAL_USART_SetCallback(HAL_USART_t* usart, HAL_USART_CallbackIdx_t cb_idx, Callback_t* callback)
{
    if(cb_idx < __NOT_CALLBACK_USART_MAX)
        usart->USART_Callbacks[cb_idx] = callback;
}

bool HAL_USART_WriteByte_Polling(const HAL_USART_t* usart, uint8_t data)
{
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsTxEnabled(usart)==false ||
        HAL_USART_IsTxDmaEnabled(usart) || 
        HAL_USART_IsTxStreamEnabled(usart) )
        return false;

    USART_SendData(usart->USARTx, data);
    while(USART_GetFlagStatus(usart->USARTx, USART_FLAG_TC) == RESET);

    return true;
}

bool HAL_USART_WriteByte(const HAL_USART_t* usart, uint8_t data)
{
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsTxEnabled(usart)==false ||
        HAL_USART_IsTxDmaEnabled(usart) )
        return false;
    
    bool ret = Buffer_Queue_Push_uint8_t(usart->USART_Tx_Buf, data);
    if(HAL_USART_IsTransmitting(usart)==false) 
        HAL_USART_TxStreamCmd(usart,true);
    return ret;
}

uint16_t HAL_USART_Write(const HAL_USART_t* usart, uint8_t* data, uint16_t len)
{
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsTxEnabled(usart)==false ||
        HAL_USART_IsTxDmaEnabled(usart) )
        return 0;
    
    if(len)
    {
        uint16_t ret = Buffer_Queue_PushArray_uint8_t(usart->USART_Tx_Buf, data, len);
        if(HAL_USART_IsTransmitting(usart)==false) 
            HAL_USART_TxStreamCmd(usart,true);
        return ret;
    }
    return 0;
}

bool HAL_USART_ReadByte_Polling(const HAL_USART_t* usart, uint8_t* data)
{
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsRxEnabled(usart)==false ||
        HAL_USART_IsRxDmaEnabled(usart) || 
        HAL_USART_IsRxStreamEnabled(usart) )
        return false;

    while(USART_GetFlagStatus(usart->USARTx, USART_FLAG_RXNE) == RESET);
    *data = USART_ReceiveData(usart->USARTx);
    return true;
}

bool HAL_USART_ReadByte(const HAL_USART_t* usart, uint8_t* data)
{
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsRxEnabled(usart)==false ||
        HAL_USART_IsRxDmaEnabled(usart) )
        return false;
    else
        return Buffer_Queue_Pop_uint8_t(usart->USART_Rx_Buf, data);
}

uint16_t HAL_USART_Read(const HAL_USART_t* usart, uint8_t* data, uint16_t len)
{
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsRxEnabled(usart)==false ||
        HAL_USART_IsRxDmaEnabled(usart) )
        return 0;
    else
        return Buffer_Queue_PopArray_uint8_t(usart->USART_Rx_Buf, data, len);
}

HAL_USART_Status_t HAL_USART_TxStreamCmd(const HAL_USART_t* usart, bool en)
{
    USART_TypeDef *usart_hw = usart->USARTx;

    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsTxEnabled(usart)==false)
        return HAL_USART_DISABLED;
    if(HAL_USART_IsTransmitting(usart)) 
        return HAL_USART_BUSY;

    if (en)
    {
        if(usart->USART_Tx_Buf == NULL)
            return HAL_USART_BUF_ERROR;

        //TC only used for DMA mode, so disable it
        //TxStream mode use TXE interrupt and 
        //Normally USART_IT_TC is disabled if we come here
        //Disable again just for robustness
        USART_ITConfig(usart_hw, USART_IT_TC, DISABLE);
        USART_ClearITPendingBit(usart_hw, USART_IT_TC);
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
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsRxEnabled(usart)==false)
        return HAL_USART_DISABLED;
    if(HAL_USART_IsRxDmaEnabled(usart)) 
        return HAL_USART_BUSY;
    else if (en)
    {
        if(usart->USART_Rx_Buf == NULL)
            return HAL_USART_BUF_ERROR;
        //IDLE only used for DMA mode, so disable it
        //RxStream mode use RXNE interrupt and _last_rx_time to detect idle
        //Normally USART_IT_IDLE is disabled if we come here
        //Disable again just for robustness
        USART_ITConfig(usart->USARTx, USART_IT_IDLE, DISABLE);
        USART_ClearITPendingBit(usart_hw, USART_IT_RXNE);//clear RXNE flag
        USART_ITConfig(usart_hw, USART_IT_RXNE, ENABLE);
    }
    else
    {
        USART_ITConfig(usart_hw, USART_IT_RXNE, DISABLE);
        USART_ClearITPendingBit(usart_hw, USART_IT_RXNE);
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

/**
 * @brief  Transfer all data in tx queue using DMA.
 * @details Callback will be invoked when tx finish.
 *          And tx DMA will be DISABLE, call again for next DMA transfer.
 * @warning Tx stream will be disabled while DMA ongoing.
 * @param  usart: usart handle
 * @retval HAL_USART_Status_t
 */
HAL_USART_Status_t HAL_USART_DmaWrite(const HAL_USART_t* usart)
{
    const HAL_DMA_t *dma_cfg = usart->USART_TxDma_Cfg;
    Buffer_uint8_t *queue = usart->USART_Tx_Buf;
    USART_TypeDef *hw_USART = usart->USARTx;

    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsTxEnabled(usart)==false)
        return HAL_USART_DISABLED;
    else if(HAL_USART_IsTransmitting(usart)) 
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
        HAL_DMA_ReloadCfg(dma_cfg);//clear dma flags manually
        HAL_DMA_SetMemAddr(dma_cfg, queue->buf_ptr);
        HAL_DMA_SetPeriphAddr(dma_cfg , &hw_USART->DR);
        HAL_DMA_SetNumOfData(dma_cfg, Buffer_Queue_GetSize(queue));
        HAL_DMA_Cmd(dma_cfg,true);

        //TXE is only used for stream mode, so disable it
        //Enable TC interrupt to detect tx dma finish
        USART_ITConfig(hw_USART, USART_IT_TXE, DISABLE);
        USART_ClearITPendingBit(hw_USART, USART_IT_TC);
        USART_ITConfig(hw_USART, USART_IT_TC, ENABLE);
        
        USART_DMACmd(hw_USART, USART_DMAReq_Tx, ENABLE);
    }

    return HAL_USART_OK;
}

/**
 * @brief  Read data using DMA, data will be stored in rx queue.
 * @details Callback will be invoked when IDLE or rx count reach len.
 *          And rx DMA will be DISABLE, call again for next DMA read.
 * @warning Rx Stream will be disabled while DMA ongoing.
 * @param  usart: usart handle
 * @param  len: number of bytes to read, 0 for max capacity of rx queue
 * @retval HAL_USART_Status_t
 */
HAL_USART_Status_t HAL_USART_DmaRead(const HAL_USART_t* usart,uint16_t len)
{
    Buffer_uint8_t* queue = usart->USART_Rx_Buf;
    const HAL_DMA_t *dma_cfg = usart->USART_RxDma_Cfg;
    USART_TypeDef *usart_hw = usart->USARTx;
    
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsRxEnabled(usart)==false)
        return HAL_USART_DISABLED;
    else if(HAL_USART_IsReceiving(usart)) 
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
        if(len==0)
            len = Buffer_Queue_GetMaxCapacity(queue);
        queue->w_ptr = queue->buf_ptr+len;
        //config DMA
        HAL_DMA_ReloadCfg(dma_cfg);//clear dma flags manually
        HAL_DMA_SetMemAddr(dma_cfg , queue->buf_ptr);
        HAL_DMA_SetPeriphAddr(dma_cfg ,&usart_hw->DR);
        HAL_DMA_SetNumOfData(dma_cfg ,len);
        HAL_DMA_Cmd(dma_cfg,true);

        //clear IDLE flag
        USART_GetFlagStatus(usart_hw, USART_FLAG_IDLE);
        USART_ReceiveData(usart_hw); 
        //RXNE is only used for stream mode, so disable it
        //Enable IDLE interrupt to detect rx dma finish
        USART_ITConfig(usart_hw, USART_IT_RXNE, DISABLE);
        //IDLE flag is cleared in ISR, no need to clear it here
        USART_ITConfig(usart_hw, USART_IT_IDLE, ENABLE);

        USART_DMACmd(usart_hw, USART_DMAReq_Rx, ENABLE);
    }
    return HAL_USART_OK;
}

HAL_USART_Status_t HAL_USART_TxStreamWake(const HAL_USART_t* usart)
{
    return HAL_USART_TxStreamCmd(usart,true);
}
HAL_USART_Status_t HAL_USART_TxDmaWake(const HAL_USART_t* usart)
{
    if( HAL_USART_IsEnabled(usart)==false || 
        HAL_USART_IsTxEnabled(usart)==false)
        return HAL_USART_DISABLED;
    if(HAL_USART_IsTransmitting(usart)) 
        return HAL_USART_BUSY;
    
    if(USART_GetITStatus(usart->USARTx,USART_IT_TC))
    {
        USART_ITConfig(usart->USARTx, USART_IT_TC, ENABLE);
        return HAL_USART_OK;
    }
    else
        return HAL_USART_ERROR;
}

void HAL_USART_IRQHandler(HAL_USART_t* usart)
{
    Callback_Invoke_Idx(usart,NULL,usart->USART_Callbacks,USART_CALLBACK_IRQ);

    if(USART_GetITStatus(usart->USARTx, USART_IT_TXE) != RESET)
    {
        uint8_t data;
        if(Buffer_Queue_Pop_uint8_t(usart->USART_Tx_Buf, &data))
            USART_SendData(usart->USARTx, data);
        else
        {
            USART_ITConfig(usart->USARTx, USART_IT_TXE, DISABLE);
            Callback_Invoke_Idx(usart,NULL,usart->USART_Callbacks,USART_CALLBACK_IRQ_TX_EMPTY);
        }
    }
    if(USART_GetITStatus(usart->USARTx, USART_IT_TC) != RESET)
    {
        USART_ITConfig(usart->USARTx, USART_IT_TC, DISABLE);
        if(HAL_USART_IsTxDmaEnabled(usart))
        {
            USART_DMACmd(usart->USARTx, USART_DMAReq_Tx, DISABLE);
            HAL_DMA_Cmd(usart->USART_TxDma_Cfg,false);
            Buffer_Clear(usart->USART_Tx_Buf);
        }
        Callback_Invoke_Idx(usart,NULL,usart->USART_Callbacks,USART_CALLBACK_IRQ_TX_EMPTY);
    }
    if(USART_GetITStatus(usart->USARTx, USART_IT_RXNE) != RESET)
    {
        //clear RXND flag and read data
        uint8_t data = USART_ReceiveData(usart->USARTx);
        if(Buffer_Queue_IsFull(usart->USART_Rx_Buf))
            Callback_Invoke_Idx(usart,NULL,usart->USART_Callbacks,USART_CALLBACK_IRQ_RX_FULL);
        
        if(Buffer_Queue_Push_uint8_t(usart->USART_Rx_Buf, data))
            usart->_last_rx_time = SysTime_Get();

        if (Buffer_Queue_GetSize(usart->USART_Rx_Buf)==usart->USART_Rx_Threshold)
            Callback_InvokeNowOrPending_Idx(usart,NULL,usart->USART_Callbacks,
                USART_CALLBACK_RX_THRSHOLD,usart->_callback_pending_flag);
    }
    if(USART_GetITStatus(usart->USARTx, USART_IT_IDLE) != RESET)
    {
        //clear IDLE flag by read data and disable IDLE interrupt
        USART_ReceiveData(usart->USARTx); 
        USART_ITConfig(usart->USARTx, USART_IT_IDLE, DISABLE);
        usart->_last_rx_time = SysTime_Get();
        if(HAL_USART_IsRxDmaEnabled(usart))
        {
            uint16_t ndt;
            USART_DMACmd(usart->USARTx, USART_DMAReq_Rx, DISABLE);
            HAL_DMA_Cmd(usart->USART_RxDma_Cfg,false);
            ndt = HAL_DMA_GetNumOfData(usart->USART_RxDma_Cfg);
            usart->USART_Rx_Buf->w_ptr -= ndt;
        }
        Callback_InvokeNowOrPending_Idx(usart,NULL,usart->USART_Callbacks,
            USART_CALLBACK_RX_TIMEOUT,usart->_callback_pending_flag);
    }
}

/**
 * @brief  USART service routine, only necessary when pending callbacks are used
 *         or stream rx timeout is enabled
 * @param  usart: pointer to a HAL_USART_t structure that contains
 *         the configuration information for the specified USART peripheral.
 * @retval None
 */
void HAL_USART_Service(HAL_USART_t* usart)
{
    //Execute pending callbacks
    while(usart->_callback_pending_flag)
    {
        uint8_t cb_idx = BitFlag_BinToIdx(usart->_callback_pending_flag);
        Callback_Invoke_Idx(usart,NULL,usart->USART_Callbacks,cb_idx);        
        BitFlag_ClearIdx(usart->_callback_pending_flag,cb_idx);
    }

    //check rx timeout, only applicable for stream rx mode
    //dma mode only use idle interrupt to detect timeout
    if( HAL_USART_IsRxDmaEnabled(usart) == false && 
        usart->USART_Rx_Timeout && Buffer_Queue_IsEmpty(usart->USART_Rx_Buf)==false &&
        (SysTime_Get() - usart->_last_rx_time) >= usart->USART_Rx_Timeout)
    {
        Callback_Invoke_Idx(usart,NULL,usart->USART_Callbacks,USART_CALLBACK_RX_TIMEOUT);   
    }
}
