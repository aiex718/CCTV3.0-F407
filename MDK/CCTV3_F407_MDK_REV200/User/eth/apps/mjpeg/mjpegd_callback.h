#ifndef __MJPEGD_TYPEDEF_H__
#define __MJPEGD_TYPEDEF_H__

#include "lwip/err.h"

typedef struct Mjpegd_Callback_s
{
    void (*func)(void* sender,void* arg,void* owner);
    void *owner;
}Mjpegd_Callback_t;

#define Mjpegd_Callback_Invoke(sender,arg,cb) do{   \
    if((cb)!=NULL&&(cb)->func!=NULL)                \
    (cb)->func((sender),(arg),(cb)->owner);         \
}while(0)   

#define Mjpegd_Callback_Invoke_Idx(sender,arg,cb_ary,cb_idx) do{  \
    Mjpegd_Callback_t* cb = (cb_ary)[(cb_idx)];                   \
    Mjpegd_Callback_Invoke((sender),(arg),cb);                    \
}while(0)

#endif
