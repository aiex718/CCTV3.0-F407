#ifndef __BUFFER_H__
#define __BUFFER_H__
#include "bsp/platform/platform_defs.h"

#ifndef BUFFER_STORAGE_LEN_T 
    #define BUFFER_STORAGE_LEN_T    uint16_t
#endif

#define __Buffer_Queue_MovePtr(self,ptr,num) do{        \
    if(((ptr)+(num))>=((self)->buf_ptr + (self)->len))  \
    {   (ptr)=(ptr)+(num)-(self)->len;               }  \
    else                                                \
    {   (ptr)=(ptr)+(num);                           }  \
}while(0)
#define __Buffer_Queue_IncPtr(self,ptr) __Buffer_Queue_MovePtr(self,ptr,1)
/* Generic decleartion  */ 
#define __BUFFER_DECL(__type,__name)                    \
__BSP_STRUCT_ALIGN typedef struct Buffer_##__name##_s   \
{                                                       \
    BUFFER_STORAGE_LEN_T      len;                      \
    __type          *buf_ptr;                           \
    __type          *r_ptr;                             \
    __type          *w_ptr;                             \
}Buffer_##__name;                                       \
/* Queue generic funstions  */                                      \
static BUFFER_STORAGE_LEN_T Buffer_Queue_PushArray_##__name         \
(Buffer_##__name *self, __type *data, BUFFER_STORAGE_LEN_T len)     \
{                                                                   \
    BUFFER_STORAGE_LEN_T w_count=0;                                 \
    if(data==NULL) return 0;                                        \
    while(len && Buffer_Queue_IsFull(self)==false)                  \
    {                                                               \
        BUFFER_STORAGE_LEN_T w_len;                                 \
        if(self->w_ptr >= self->r_ptr)                              \
            w_len = self->buf_ptr + self->len - self->w_ptr -       \
                    (self->r_ptr == self->buf_ptr ? 1 : 0);         \
        else                                                        \
            w_len = self->r_ptr - self->w_ptr - 1;                  \
                                                                    \
        w_len = BSP_MIN(w_len,len);                                 \
        BSP_MEMCPY(self->w_ptr,data,w_len*sizeof(__type));          \
        data+=w_len;                                                \
        __Buffer_Queue_MovePtr(self,self->w_ptr,w_len);             \
                                                                    \
        len-=w_len;                                                 \
        w_count+=w_len;                                             \
    }                                                               \
    return w_count;                                                 \
}                                                                   \
static BUFFER_STORAGE_LEN_T Buffer_Queue_PopArray_##__name          \
(Buffer_##__name *self, __type *data, BUFFER_STORAGE_LEN_T len)     \
{                                                                   \
    BUFFER_STORAGE_LEN_T r_count=0;                                 \
    if(data==NULL) return 0;                                        \
    while(len && Buffer_Queue_IsEmpty(self)==false)                 \
    {                                                               \
        BUFFER_STORAGE_LEN_T r_len;                                 \
        if(self->r_ptr >= self->w_ptr)                              \
            r_len = self->buf_ptr + self->len - self->r_ptr;        \
        else                                                        \
            r_len = self->w_ptr - self->r_ptr;                      \
                                                                    \
        r_len = BSP_MIN(r_len,len);                                 \
        BSP_MEMCPY(data,self->r_ptr,r_len*sizeof(__type));          \
        data+=r_len;                                                \
        __Buffer_Queue_MovePtr(self,self->r_ptr,r_len);             \
                                                                    \
        len-=r_len;                                                 \
        r_count+=r_len;                                             \
    }                                                               \
    return r_count;                                                 \
}                                                                   \
__STATIC_INLINE bool Buffer_Queue_Push_##__name                     \
(Buffer_##__name *self, __type x)                                   \
{                                                                   \
    if(Buffer_Queue_IsFull(self))                                   \
        return false;                                               \
    Buffer_Queue_QuickPush(self,x);                                 \
    return true;                                                    \
}                                                                   \
__STATIC_INLINE bool Buffer_Queue_Pop_##__name                      \
(Buffer_##__name *self, __type *o)                                  \
{                                                                   \
    if(Buffer_Queue_IsEmpty(self)) return false;                    \
    Buffer_Queue_QuickPop(self,*o);                                 \
    return true;                                                    \
}                                                                   \
__STATIC_INLINE bool Buffer_Queue_IndexOf_##__name                  \
(Buffer_##__name *self, __type data, BUFFER_STORAGE_LEN_T *idx_out) \
{                                                                   \
    BUFFER_STORAGE_LEN_T idx = 0;                                   \
    __type *ptr = self->r_ptr;                                      \
    while(ptr != self->w_ptr)                                       \
    {                                                               \
        if(*ptr == data)                                            \
        {                                                           \
            *idx_out = idx;                                         \
            return true;                                            \
        }                                                           \
        __Buffer_Queue_IncPtr(self,ptr);                            \
        idx++;                                                      \
    }                                                               \
    return false;                                                   \
}                                                                   \
/* Stack generic funstions  */                                      \
static BUFFER_STORAGE_LEN_T Buffer_Stack_PushArray_##__name         \
(Buffer_##__name *self, __type *data, BUFFER_STORAGE_LEN_T len)     \
{                                                                   \
    BUFFER_STORAGE_LEN_T cnt=0;                                     \
    if(data==NULL) return 0;                                        \
    cnt=Buffer_Stack_GetSize(self);                                 \
    while(len-- && Buffer_Stack_IsFull(self)==false)                \
        Buffer_Stack_QuickPush(self,*data++);                       \
    return Buffer_Stack_GetSize(self)-cnt;                          \
}                                                                   \
static BUFFER_STORAGE_LEN_T Buffer_Stack_PopArray_##__name          \
(Buffer_##__name *self, __type *data, BUFFER_STORAGE_LEN_T len)     \
{                                                                   \
    BUFFER_STORAGE_LEN_T cnt=0;                                     \
    if(data==NULL) return 0;                                        \
    cnt=Buffer_Stack_GetSize(self);                                 \
    while(len-- && Buffer_Stack_IsEmpty(self)==false)               \
        Buffer_Stack_QuickPop(self,*data++);                        \
    return cnt-Buffer_Stack_GetSize(self);                          \
}                                                                   \
__STATIC_INLINE bool Buffer_Stack_Push_##__name                     \
(Buffer_##__name *self, __type x)                                   \
{                                                                   \
    if(Buffer_Stack_IsFull(self))   return false;                   \
    Buffer_Stack_QuickPush(self,x); return true;                    \
}                                                                   \
__STATIC_INLINE bool Buffer_Stack_Pop_##__name                      \
(Buffer_##__name *self, __type *o)                                  \
{                                                                   \
    if(Buffer_Stack_IsEmpty(self))  return false;                   \
    Buffer_Stack_QuickPop(self,*o); return true;                    \
}


//Queue and stack mode shared functions
#define Buffer_GetStorageSize(self) ((self)->len)
#define Buffer_Init(self,_buf,_len) \
    do{(self)->w_ptr=(self)->r_ptr=(self)->buf_ptr=(_buf);(self)->len=(_len);}while(0)
#define Buffer_Clear(self) do{(self)->w_ptr=(self)->r_ptr=(self)->buf_ptr;}while(0)

//Queue functions
#define Buffer_Queue_IsEmpty(self)  ((self)->r_ptr == (self)->w_ptr)
#define Buffer_Queue_IsFull(self)   (((self)->r_ptr == (self)->w_ptr + 1) ||   \
    (((self)->r_ptr == (self)->buf_ptr) && ((self)->w_ptr == ((self)->buf_ptr + (self)->len - 1))))
#define Buffer_Queue_GetSize(self) \
    (((self)->w_ptr >= (self)->r_ptr)?((self)->w_ptr - (self)->r_ptr):((self)->w_ptr - (self)->r_ptr + (self)->len))
#define Buffer_Queue_QuickPush(self,x) do{  \
    *((self)->w_ptr)=(x);__Buffer_Queue_IncPtr((self),(self)->w_ptr);  }while(0)
#define Buffer_Queue_QuickPop(self,o)  do{  \
    (o)= *((self)->r_ptr);__Buffer_Queue_IncPtr((self),(self)->r_ptr); }while(0)
#define Buffer_Queue_Remove(self,cnt)  do{  \
    BUFFER_STORAGE_LEN_T i=cnt;             \
    while(i--) __Buffer_Queue_IncPtr((self),(self)->r_ptr); }while(0)
#define Buffer_Queue_GetMaxCapacity(self)   (Buffer_GetStorageSize(self) - 1)
#define Buffer_Queue_GetCapacity(self)      (Buffer_Queue_GetMaxCapacity(self)-Buffer_Queue_GetSize(self))

//Stack functions
#define Buffer_Stack_IsEmpty(self)  ((self)->buf_ptr == (self)->w_ptr)
#define Buffer_Stack_IsFull(self)   ((self)->buf_ptr + (self)->len == (self)->w_ptr)
#define Buffer_Stack_GetSize(self)  ((self)->w_ptr - (self)->buf_ptr)
#define Buffer_Stack_QuickPush(self,x) do{*((self)->w_ptr)=(x); (self)->w_ptr++;}while(0)
#define Buffer_Stack_QuickPop(self,o)  do{(self)->w_ptr--;(o)= *((self)->w_ptr);}while(0)
#define Buffer_Stack_GetMaxCapacity(self) Buffer_GetStorageSize(self)
#define Buffer_Stack_GetCapacity(self) (Buffer_Stack_GetMaxCapacity(self)-Buffer_Stack_GetSize(self))


__BUFFER_DECL(uint8_t,uint8_t)
__BUFFER_DECL(uint16_t,uint16_t)
__BUFFER_DECL(uint32_t,uint32_t)

#endif
