#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/buffer.h"

//CallbackIT is always invoked
typedef struct
{
    void (*func)(void* sender,void* param);
    void *param;
}CallbackIT_t;

//Callback_t is (selectable) invoked in interrupt or delayed to task/main thread
typedef struct
{
    void (*func)(void* sender,void* param);
    void *param;
    bool inIRq;
}Callback_t;

__BUFFER_DECL(Callback_t*,CallbackP_t)

#define Callback_Invoke(sender,cb) do{if(cb!=NULL) cb->func(sender,cb->param);}while(0)   

/**
 * @brief invoke callback or queue it 
 * @param sender: sender of callback
 * @param queue: queue to store callback
 * @param cb: callback to invoke or queue
 * @return true if the callback is queued, false if the callback is invoked
 */
__STATIC_INLINE bool Callback_InvokeOrQueue_IRq(void *sender,
    Callback_t *cb, Buffer_CallbackP_t *queue)
{
    if(cb!=NULL)
    {
        if(cb->inIRq)
            Callback_Invoke(sender,cb);
        else
        {
            Buffer_Queue_Push_CallbackP_t(queue,cb);
            return true;
        }
    }
    return false;
}


#endif
