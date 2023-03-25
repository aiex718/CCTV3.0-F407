#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include "bsp/platform/platform_defs.h"

// CONCURRENT_QUEUE_INDEX_T data type should be as large as possible
// these index is immune to overflow, but could encounter ABA problem
// if set to small type like uint8_t
#ifndef CONCURRENT_QUEUE_INDEX_T 
    #define CONCURRENT_QUEUE_INDEX_T    uint32_t
#endif

#ifndef CONCURRENT_QUEUE_STORAGE_T 
    #define CONCURRENT_QUEUE_STORAGE_T    uint8_t
#endif

#ifndef CONCURRENT_QUEUE_STORAGE_LEN_T 
    #define CONCURRENT_QUEUE_STORAGE_LEN_T    uint16_t
#endif

#ifndef CONCURRENT_QUEUE_ALLOW_PREEMPT 
    #define CONCURRENT_QUEUE_ALLOW_PREEMPT    0
#endif

#define __CONCURRENT_QUEUE_MACRO_CAT(a,b) a##b
#define __CONCURRENT_QUEUE_TYPE_GENERIC(__type)     \
    __CONCURRENT_QUEUE_MACRO_CAT(Concurrent_Queue_,__type)
#define __CONCURRENT_QUEUE_T    \
    __CONCURRENT_QUEUE_TYPE_GENERIC(CONCURRENT_QUEUE_STORAGE_T)
                     
__BSP_STRUCT_ALIGN typedef struct                       
{                                                       
    __IO CONCURRENT_QUEUE_INDEX_T r_idx,w_idx,l_idx;    
    CONCURRENT_QUEUE_STORAGE_T *mem;             
    CONCURRENT_QUEUE_STORAGE_LEN_T mem_len;           
}__CONCURRENT_QUEUE_T;

void Concurrent_Queue_Init(__CONCURRENT_QUEUE_T * self,         \
                           CONCURRENT_QUEUE_STORAGE_T * buf,    \
                           CONCURRENT_QUEUE_STORAGE_LEN_T buf_len);
void Concurrent_Queue_Clear(__CONCURRENT_QUEUE_T *self);
bool Concurrent_Queue_TryPush(__CONCURRENT_QUEUE_T *self, \
                              CONCURRENT_QUEUE_STORAGE_T val);
bool Concurrent_Queue_TryPop(__CONCURRENT_QUEUE_T *self, \
                             CONCURRENT_QUEUE_STORAGE_T *out);


#endif

