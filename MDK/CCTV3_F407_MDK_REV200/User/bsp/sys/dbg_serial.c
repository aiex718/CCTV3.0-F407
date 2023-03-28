#include "bsp/sys/dbg_serial.h"
#include "bsp/platform/periph_list.h"
#include "bsp/hal/usart.h"
#include "bsp/sys/sysctrl.h"
#include "stdio.h"

void DBG_Serial_UsartTxEmptyCallback(void *sender,void *arg)
{
    HAL_USART_t *usart = (HAL_USART_t*)sender;
    DBG_Serial_t *self = (DBG_Serial_t*)arg;
    uint8_t tmp;

    Buffer_Clear(usart->USART_Tx_Buf);
    while ( Buffer_Queue_IsFull(usart->USART_Tx_Buf)==false && 
            Concurrent_Queue_TryPop(self->tx_con_queue,&tmp) )
    {
        Buffer_Queue_Push_uint8_t(usart->USART_Tx_Buf,tmp);
    }
    if(Buffer_Queue_IsEmpty(usart->USART_Tx_Buf)==false)
    {
#if DBG_SERIAL_ENABLE_DMA
        HAL_USART_DmaWrite(usart);
#else
        HAL_USART_TxStreamCmd(usart,true);
#endif
    }
}

void DBG_Serial_UsartRxTimeoutCallback(void *sender,void *arg)
{
    HAL_USART_t *usart = (HAL_USART_t*)sender;
    DBG_Serial_t *self = (DBG_Serial_t*)arg;
    uint8_t tmp, buf[Debug_Serial_Rx_Buffer_Size]={0};
    uint8_t *r_ptr, *w_ptr = buf;

    while (Buffer_Queue_Pop_uint8_t(usart->USART_Rx_Buf,&tmp))
    {
        //fetching data, drop if rxbuf full
        //only push to rxbuf when full line is received
        *w_ptr++ = tmp;
        if(tmp=='\n')
        {
            if(Buffer_Queue_GetCapacity(self->rx_buf)>=w_ptr-buf)
            {
                for(r_ptr=buf;r_ptr<w_ptr;r_ptr++)
                    Buffer_Queue_Push_uint8_t(self->rx_buf,*r_ptr);
            }
            w_ptr = buf;
        }
    }
#if DBG_SERIAL_ENABLE_DMA
        HAL_USART_DmaRead(usart,0);
#endif    
}

void DBG_Serial_Init(DBG_Serial_t *self)
{   
    if(self->tx_con_queue!=NULL)
        Concurrent_Queue_Clear(self->tx_con_queue);
    if(self->rx_buf!=NULL)
        Buffer_Clear(self->rx_buf);

    if(self->_tx_empty_cb)
    {
        self->_tx_empty_cb->func = DBG_Serial_UsartTxEmptyCallback;
        self->_tx_empty_cb->param = self;
        self->_tx_empty_cb->IRqCfg = INVOKE_IN_IRQ;
    }

    if(self->_rx_timeout_cb)
    {
        self->_rx_timeout_cb->func = DBG_Serial_UsartRxTimeoutCallback;
        self->_rx_timeout_cb->param = self;
        self->_tx_empty_cb->IRqCfg = INVOKE_IN_IRQ;
    }
}

void DBG_Serial_AttachUSART(DBG_Serial_t *self, HAL_USART_t *usart)
{
    if(usart)
    {
        self->usart = usart;
        HAL_USART_Init(self->usart);    
        HAL_USART_SetCallback(self->usart,USART_CALLBACK_TX_EMPTY,self->_tx_empty_cb);
        HAL_USART_SetCallback(self->usart,USART_CALLBACK_RX_TIMEOUT,self->_rx_timeout_cb);
#if DBG_SERIAL_ENABLE_DMA
	    HAL_USART_DmaRead(self->usart,0);
#else
        HAL_USART_RxStreamCmd(self->usart,true);
#endif
    }
}

uint16_t DBG_Serial_ReadLine(DBG_Serial_t *self,uint8_t* buf, uint16_t buf_len)
{
    uint16_t idx;
    if(Buffer_Queue_IndexOf_uint8_t(self->rx_buf,'\n',&idx) && buf_len>idx)
    {
        buf_len=idx;
        while (buf_len--)        
            Buffer_Queue_Pop_uint8_t(self->rx_buf,buf++);
        Buffer_Queue_Remove(self->rx_buf,1);//remove last '\n'
        return idx;
    }
    return 0;
}

void DBG_Serial_Service(DBG_Serial_t *self)
{
    //Invoke callback manually if tx is not busy
    if(self->usart && HAL_USART_IsTransmitting(self->usart)==false)
        DBG_Serial_UsartTxEmptyCallback(self->usart,self);
}

//Link to stdio
int fputc(int ch, FILE *f)
{
    //we set USART NVIC pirority to 0.2 in this demo, if printf is called 
    //in other IRq whose pirority is lower than 0.2, and DBG_Serial->tx_con_queue
    //is full or capacity is not enough, It'll cause deadlock.
    //you can use SysCtrl_IsThreadInIRq() to check if it's in IRq,
    //if in IRq, break the loop, but you'll lose some data.
	while (Concurrent_Queue_TryPush(DBG_Serial->tx_con_queue,(uint8_t)ch)==false);
	return (ch);
}


int fgetc(FILE *f)
{
	uint8_t ch=0;
	while (Buffer_Queue_Pop_uint8_t(DBG_Serial->rx_buf,&ch)==false);
	return (int)ch;
}

