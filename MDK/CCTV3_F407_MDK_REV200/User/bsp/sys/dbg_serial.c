#include "bsp/sys/dbg_serial.h"
#include "bsp/platform/periph_list.h"
#include "bsp/hal/usart.h"
#include "bsp/sys/sysctrl.h"
#include "stdio.h"
/*
This module is designed to redirect printf to usart, and make printf 
thread-safe so we can call printf everywhere including ISR, but beaware
that DATA WILL BE DROPPED if tx queue is full when using printf in ISR.
*/
void DBG_Serial_UsartTxEmptyCallback(void *sender,void *arg,void* owner)
{
    HAL_USART_t *hal_usart = (HAL_USART_t*)sender;
    DBG_Serial_t *self = (DBG_Serial_t*)owner;
    uint8_t tmp;

    Buffer_Clear(hal_usart->USART_Tx_Buf);
    while ( Buffer_Queue_IsFull(hal_usart->USART_Tx_Buf)==false && 
            Concurrent_Queue_TryPop(self->tx_con_queue,&tmp) )
    {
        Buffer_Queue_Push_uint8_t(hal_usart->USART_Tx_Buf,tmp);
    }

    if(Buffer_Queue_IsEmpty(hal_usart->USART_Tx_Buf)==false)
    {
#if DBG_SERIAL_ENABLE_DMA
        HAL_USART_DmaWrite(hal_usart);
#else
        HAL_USART_TxStreamCmd(hal_usart,true);
#endif
    }
}

void DBG_Serial_UsartRxTimeoutCallback(void *sender,void *arg,void* owner)
{
    HAL_USART_t *hal_usart = (HAL_USART_t*)sender;
    DBG_Serial_t *self = (DBG_Serial_t*)owner;
    uint8_t tmp;

    while (Buffer_Queue_Pop_uint8_t(hal_usart->USART_Rx_Buf,&tmp) && 
            Concurrent_Queue_TryPush(self->rx_con_queue,tmp));
    
    if(Buffer_Queue_IsEmpty(hal_usart->USART_Rx_Buf)==false)
        printf("DBG_Serial: rx data dropped\n");

#if DBG_SERIAL_ENABLE_DMA
        HAL_USART_DmaRead(hal_usart,0);
#endif    
    //no need to restart rx stream(if enable) here, it's always on
}

void DBG_Serial_Init(DBG_Serial_t *self)
{
    if(self->tx_con_queue!=NULL)
        Concurrent_Queue_Clear(self->tx_con_queue);
    if(self->rx_con_queue!=NULL)
        Concurrent_Queue_Clear(self->rx_con_queue);

    self->_tx_empty_cb.func = DBG_Serial_UsartTxEmptyCallback;
    self->_tx_empty_cb.owner = self;
#if DBG_SERIAL_USING_USART_ISR
    self->_tx_empty_cb.invoke_cfg = INVOKE_IMMEDIATELY;
#else
    self->_tx_empty_cb->invoke_cfg = INVOKE_IN_SERVICE;
#endif

    self->_rx_timeout_cb.func = DBG_Serial_UsartRxTimeoutCallback;
    self->_rx_timeout_cb.owner = self;
#if DBG_SERIAL_USING_USART_ISR
    self->_rx_timeout_cb.invoke_cfg = INVOKE_IMMEDIATELY;
#else
    self->_rx_timeout_cb.invoke_cfg = INVOKE_IN_SERVICE;
#endif

    if(self->hal_usart!=NULL)
        DBG_Serial_AttachUSART(self,self->hal_usart);
    
}

void DBG_Serial_AttachUSART(DBG_Serial_t *self, HAL_USART_t *hal_usart)
{
    if(hal_usart)
    {
        self->hal_usart = hal_usart;
        HAL_USART_Init(self->hal_usart);    
        HAL_USART_SetCallback(self->hal_usart,
            USART_CALLBACK_IRQ_TX_EMPTY,&self->_tx_empty_cb);
        HAL_USART_SetCallback(self->hal_usart,
            USART_CALLBACK_RX_TIMEOUT,&self->_rx_timeout_cb);

#if DBG_SERIAL_ENABLE_DMA
	    HAL_USART_DmaRead(self->hal_usart,0);
#else
        HAL_USART_RxStreamCmd(self->hal_usart,true);
#endif
    }
}

void DBG_Serial_Cmd(DBG_Serial_t *self,bool en)
{
    HAL_USART_Cmd(self->hal_usart,en);

    if(en==false)
    {
        if(self->tx_con_queue!=NULL)
            Concurrent_Queue_Clear(self->tx_con_queue);
        if(self->rx_con_queue!=NULL)
            Concurrent_Queue_Clear(self->rx_con_queue);
    }
}

/*
Debug serial needs to trigger transfer manually when idle,
the task is wrapped in DBG_Serial_Service().
Beaware this service IS NOT THREAD SAFE due to usart->USART_Tx_Buf.
And because different ISR(using printf) may preempt each other, we can't 
put DBG_Serial_Service in fputc. It'll cause reentrancy/thread-safe issue.

There's several ways to trigger transfer manually:
1. Dedicate software thread/task with os or task scheduler.
  - If we have os/task-scheduler, put DBG_Serial_Service in a dedicate 
    thread/task, and configure callback to delay invoke to service,
        (i.g. set self->_tx_empty_cb->invoke_cfg = INVOKE_IN_SERVICE)
        (and self->_rx_timeout_cb->invoke_cfg = INVOKE_IN_SERVICE)
    In this approach, all callback are invoked in the same service 
    thread/task to avoid reentrancy.
  - It's EXPECTED all tasks(task using printf and DBG_Serial_Service)
    execute periodically, NO STARVATION IS ALLOWED.
    Debug serial use a thread-safe queue(concurrent_queue.h) to store data.
    It's preemptible when pushing data, but not preemptible when pop data.
    Please refer to concurrent_queue.c/.h for more info.

2. Dedicate hardware timer.
  - If we're using bare metal system, we can't just put DBG_Serial_Service
    in main loop, when using printf in main/task, it'll keep retry when
    tx queue is full, which block DBG_Serial_Service then cause deadlock.
  - We can use a hardware timer ISR to call DBG_Serial_Service periodically.
    Callback should be configure to delay invoke to service(same as 1).
    In this approach, all callback are invoked in hardware timer ISR.
  - Starvation won't happen in this method.
    We have no schduler, all task running in main loop won't preempt each 
    other, task is guaranteed to finished. If preempt by an ISR, it'll 
    eventually go back to task because printf in ISR is non-blocking.

3. Only use USART ISR (default setting).
  - Peiodically calling DBG_Serial_Service is NOT NEEDED and NOT ALLOWED.
  - We can manully trigger UsartTxEmptyCallback by toggle corresponding 
    interrupt enable bit, it's wrapped by TxStream/TxDma wake function.
    All callbacks have to configure to invoke immediately.
        (i.g. set self->_tx_empty_cb->invoke_cfg = INVOKE_IMMEDIATELY)
        (and self->_rx_timeout_cb->invoke_cfg = INVOKE_IMMEDIATELY)
  - In this approach, callback is only invoked in USART ISR.

  - It's applicable in both bare metal and os/task-scheduler.
    But still NO STARVATION IS ALLOWED.
    If used with bare metal, it's same as 2 so everything is fine.
    If used with os/task-scheduler, it's same as 1, make sure no task 
    is starved.
*/
void DBG_Serial_Service(DBG_Serial_t *self)
{
    HAL_USART_t *usart = self->hal_usart;
#if DBG_SERIAL_USING_USART_ISR
    BSP_UNUSED_ARG(self);
#else
    if(usart != NULL && HAL_USART_IsEnabled(usart))
    {
        HAL_USART_Service(usart);
        //Invoke callback manually if tx is idle to trigger transfer
        if(HAL_USART_IsTransmitting(usart)==false)
            DBG_Serial_UsartTxEmptyCallback(usart,self);
    }
#endif
}

uint16_t DBG_Serial_ReadLine(DBG_Serial_t *self,uint8_t* buf, 
    uint16_t buf_len)
{
    uint8_t tmp, *w_ptr = buf;
    while(Concurrent_Queue_TryPop(self->rx_con_queue,&tmp) && 
           (w_ptr - buf) < buf_len)
    {
        if(tmp != '\n')
            *w_ptr++ = tmp;
        else
            return w_ptr - buf;
    }
    return 0;//only return len when '\n' is received
}

/*
Due to the limitations of the concurrent_queue's preemption mechanism, 
the fputc function has been designed as follows:
  - If using printf in task/main loop.
    fputc will keep retrying push data to tx queue when it's full.
    i.e. Calling printf in task/main is blocking when tx queue is full.

  - If using printf in ISR 
    fputc WILL NOT retry push data to tx queue, if queue is full, it'll 
    drop data.
    i.e. Calling printf in ISR is non-blocking.

For example, if we have task A and task B both using printf, and a service 
task executing DBG_Serial_Service function.

Suppose task A preempted by task B during printf.

1.  If task B is a ISR, it'll giveup pushing data to tx queue when it's 
full, then return to task A and let it finish pushing.
Service task can pop data from tx queue after task A.

2.  If task B is another task.
  - If task A preempted before push finished, and the tx queue is full so 
    task B is retrying, and schduler switch to service task at this moment, 
    the service task will not be able to pop data from tx queue now because
    task A is still pushing data.
    Schduler is expected to switch back to task A eventually, allow it 
    finish pushing data. Once all data has been pushed, service task is 
    able to pop data from the queue.

  - If task A preempted after push finished, and the tx queue is full so 
    task B is retrying, and schduler switch to service task at this moment,
    service task can pop data from tx queue now, because all data is pushed.
*/
int fputc(int ch, FILE *f)
{
    HAL_USART_t *usart = DBG_Serial->hal_usart;
    if(usart != NULL && HAL_USART_IsEnabled(usart))
    {
        while( 
            Concurrent_Queue_TryPush(DBG_Serial->tx_con_queue,(uint8_t)ch)==false && 
            SysCtrl_IsThreadInIRq() == false );
                //yield();
#if DBG_SERIAL_USING_USART_ISR
    #if DBG_SERIAL_ENABLE_DMA
            HAL_USART_TxStreamWake(usart);
    #else
            HAL_USART_TxDmaWake(usart);
    #endif
#endif
    }
	return (ch);
}

int fgetc(FILE *f)
{
	uint8_t ch=0;
    while (Concurrent_Queue_TryPop(DBG_Serial->rx_con_queue,&ch) == false);
	return (int)ch;
}

