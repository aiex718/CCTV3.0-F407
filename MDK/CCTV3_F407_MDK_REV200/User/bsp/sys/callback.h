#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/buffer.h"

typedef enum {
    INVOKE_IN_TASK=0,
    INVOKE_IN_IRQ=1,
}Callback_IRqCfg_t;

//CallbackIT is always invoked, no matter in interrupt or task/main thread
__BSP_STRUCT_ALIGN typedef struct
{
    void (*func)(void* sender,void* param);
    void *param;
}CallbackIRq_t;

//Callback_t is (selectable) invoked in interrupt or delayed to task/main thread
//due to callback issuer, it might not be invoked as IRQCFG settingm
__BSP_STRUCT_ALIGN typedef struct
{
    void (*func)(void* sender,void* param);
    void *param;
    Callback_IRqCfg_t IRqCfg;
}Callback_t;

__BUFFER_DECL(Callback_t*,CallbackP_t)

#define Callback_Invoke(sender,cb) do{ \
    if((cb)!=NULL) (cb)->func((sender),(cb)->param); }while(0)   

// #define Callback_InvokeOrQueue_Auto(sender,cb,queue) \
//     Callback_InvokeOrQueue((sender),(cb),(queue),SysCtrl_IsThreadInIRq())
#define Callback_InvokeOrQueue_IRq(sender,cb,queue) \
    Callback_InvokeOrQueue((sender),(cb),(queue),INVOKE_IN_IRQ)
/**
 * @brief invoke callback or queue it, invoked directly if queue is full
 * @details Normally we only queue callback in IRQ, which qualvient to 
 *        1 producer 1 consumer(task or thread) situation, so thread-safe queue 
 *        is not necessary here, but if your queue is not thread-safe in this 
 *        situation, for example, your cpu cant handle atomic pointer assignment,
 *        you have to use thread-safe or concurrent queue.
 * @param sender: sender of callback
 * @param queue: queue to store callback
 * @param cb: callback to invoke or queue
 * @return true if the callback is invoked, false if the callback is queued or null
 */
__STATIC_INLINE bool Callback_InvokeOrQueue(void *sender,
    Callback_t *cb, Buffer_CallbackP_t *queue,Callback_IRqCfg_t irqcfg)
{
    if(cb!=NULL && (
            cb->IRqCfg == irqcfg || 
            Buffer_Queue_Push_CallbackP_t(queue,cb)==false)
        )
    {
        Callback_Invoke(sender,cb);
        return true;
    }
    else
        return false;
}


#endif
