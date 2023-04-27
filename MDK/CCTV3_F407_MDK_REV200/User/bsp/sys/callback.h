#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/buffer.h"

typedef enum {
    INVOKE_IN_SERVICE = 0,
    INVOKE_IMMEDIATELY,
}Callback_InvokeCfg_t;

//Callback_t is configurable to invoke immediately(normally from a ISR)
//or in service function(normally from a task or main thread)
//depend on callback issuer, config is not always fulfilled
__BSP_STRUCT_ALIGN typedef struct Callback_s
{
    void (*func)(void* sender,void* arg,void* owner);
    void *owner;
    Callback_InvokeCfg_t invoke_cfg;
}Callback_t;

__BUFFER_DECL(Callback_t*,CallbackP_t)

//Macro as function
#define Callback_Invoke(sender,arg,cb) do{  \
    if((cb)!=NULL&&(cb)->func!=NULL)        \
    (cb)->func((sender),(arg),(cb)->owner); \
}while(0)   

#define Callback_Invoke_Idx(sender,arg,cb_ary,cb_idx) do{   \
    Callback_t* cb = (cb_ary)[(cb_idx)];                    \
    Callback_Invoke((sender),(arg),cb);                     \
}while(0)

#define Callback_InvokeNowOrPending_Idx(sender,arg,cb_ary,cb_idx,pendflag) do{  \
    Callback_t* cb = (cb_ary)[(cb_idx)];                                        \
    if(cb!=NULL)                                                                \
    {                                                                           \
        if(cb->invoke_cfg == INVOKE_IMMEDIATELY)                                \
            Callback_Invoke((sender),(arg),(cb));                               \
        else                                                                    \
            BitFlag_SetIdx((pendflag),(cb_idx));                                \
    }                                                                           \
}while(0)

/**
 * @brief try invoke callback when cfg match cb->invoke_cfg
 * @param sender: sender of callback
 * @param cb: callback to invoke
 * @return true if callback is invoked, false if not invoked
 */
__STATIC_INLINE bool Callback_TryInvoke(void *sender,void *arg,
    Callback_t *cb, Callback_InvokeCfg_t cfg)
{
    if(cb->invoke_cfg == cfg)
    {
        Callback_Invoke(sender,arg,cb);
        return true;
    }
    else
        return false;
}

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
 * @return true if callback is invoked, false if the callback is queued 
 */
__STATIC_INLINE bool Callback_InvokeOrQueue(void *sender, void* arg,
    Callback_t *cb, Buffer_CallbackP_t *queue,Callback_InvokeCfg_t irqcfg)
{
    if( cb->invoke_cfg == irqcfg || queue==NULL ||
        Buffer_Queue_Push_CallbackP_t(queue,cb)==false )
    {
        Callback_Invoke(sender,arg,cb);
        return true;
    }
    else
        return false;
}


#endif
