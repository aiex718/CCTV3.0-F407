#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include "bsp/platform/platform_defs.h"
/*
This is a array-based thread-safe queue(FIFO) implementation.
The synchronization is based on atomic operation, not mutex.

In default configuration, (i.e. allow_push_preempt=false)
push-and-pop from different thread can be considered as a wait-free
operation, threads don't have to wait for each other.

Pop-and-pop from different thread can be considered as a lock-free 
operation. The second pop thread will not be blocked by the first,
the first pop will retry in a CAS loop.

Push-and-push from different thread is not wait-free nor lock-free,
the second push thread will be blocked by the first one.


In preemptible configuration, (i.e. allow_push_preempt=true)
Push-and-push from different thread can be considered as a wait-free
operation, the second push thread don't have to wait for the first one.

But pop operation will be blocked until all push operation are done,
so pop-and-pop or push-and-pop from different thread are blocking.

It's not a lock-free queue overall, but can be a lock-free queue 
when allow_push_preempt=false and only one thread is pushing data.
*/


/*
The CONCURRENT_QUEUE_INDEX_T data type should be unsigned and 
your cpu architecture must be able to access it atomically,
otherwise interger atomic operation/library is needed.

Also it should be as large as possible to avoid ABA problem,
dont use uint8_t unless your queue is very small.
*/
#ifndef CONCURRENT_QUEUE_INDEX_T 
    #define CONCURRENT_QUEUE_INDEX_T    uint32_t
#endif

#ifndef CONCURRENT_QUEUE_STORAGE_T 
    #define CONCURRENT_QUEUE_STORAGE_T    uint8_t
#endif

#ifndef CONCURRENT_QUEUE_STORAGE_LEN_T 
    #define CONCURRENT_QUEUE_STORAGE_LEN_T    uint16_t
#endif

#define __CONCURRENT_QUEUE_MACRO_CAT(a,b) a##b
#define __CONCURRENT_QUEUE_TYPE_GENERIC(__type)     \
    __CONCURRENT_QUEUE_MACRO_CAT(Concurrent_Queue_,__type)
#define __CONCURRENT_QUEUE_T    \
    __CONCURRENT_QUEUE_TYPE_GENERIC(CONCURRENT_QUEUE_STORAGE_T)

#define __CONCURRENT_QUEUE_MACRO_CAT3(a,b,c) a##b##c
#define __CONCURRENT_QUEUE_STRUCT_GENERIC(__type)     \
    __CONCURRENT_QUEUE_MACRO_CAT3(Concurrent_Queue_,__type,_s)
#define __CONCURRENT_QUEUE_S    \
    __CONCURRENT_QUEUE_STRUCT_GENERIC(CONCURRENT_QUEUE_STORAGE_T)
                     
__BSP_STRUCT_ALIGN typedef struct __CONCURRENT_QUEUE_S             
{                                                       
    __IO CONCURRENT_QUEUE_INDEX_T r_idx,w_idx,l_idx;
    //The mem length should be power of 2 
    CONCURRENT_QUEUE_STORAGE_T *mem;             
    CONCURRENT_QUEUE_STORAGE_LEN_T mem_len;
    bool allow_push_preempt;
}__CONCURRENT_QUEUE_T;

void Concurrent_Queue_Init(__CONCURRENT_QUEUE_T * self,         \
                           CONCURRENT_QUEUE_STORAGE_T * buf,    \
                           CONCURRENT_QUEUE_STORAGE_LEN_T buf_len);
void Concurrent_Queue_Clear(__CONCURRENT_QUEUE_T *self);
bool Concurrent_Queue_TryPush(__CONCURRENT_QUEUE_T *self, \
                              CONCURRENT_QUEUE_STORAGE_T val);
bool Concurrent_Queue_TryPop(__CONCURRENT_QUEUE_T *self, \
                             CONCURRENT_QUEUE_STORAGE_T *out);

__STATIC_INLINE bool Concurrent_Queue_IsEmpty(__CONCURRENT_QUEUE_T *self)
{
    return self->w_idx==self->r_idx;
}

__STATIC_INLINE bool Concurrent_Queue_IsFull(__CONCURRENT_QUEUE_T *self)
{
    return self->w_idx-self->r_idx>=self->mem_len;
}

//check if concurrent queue can push immediately without CAS loop
__STATIC_INLINE bool Concurrent_Queue_CanPushNow(__CONCURRENT_QUEUE_T *self)
{
    if(Concurrent_Queue_IsFull(self))
        return false;
    else if(self->allow_push_preempt)
        return true;
    else if (self->w_idx==self->l_idx) 
        return true;//not allow_push_preempt and all write is committed
    else
        return false;//other thread is pushing data
}

//check if concurrent queue can pop immediately without CAS loop
__STATIC_INLINE bool Concurrent_Queue_CanPopNow(__CONCURRENT_QUEUE_T *self)
{
    if(Concurrent_Queue_IsEmpty(self))
        return false;
    else if(self->allow_push_preempt==false)
        return true;//if not allow push_preempt, we always can pop immediately
    else if(self->w_idx==self->l_idx)
        return true;//allow_push_preempt and all write is committed
    else
        return false;//allow_push_preempt and other thread is pushing data
}

#endif

