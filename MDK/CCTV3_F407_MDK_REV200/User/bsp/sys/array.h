#ifndef ARRAY_H
#define ARRAY_H

#include "bsp/platform/platform_defs.h"

#define __ARRAY_LEN uint16_t

#define __ARRAY_DECL(__type)        \
__HAL_STRUCT_ALIGN typedef struct   \
{                                   \
    __type          *ptr;           \
    __ARRAY_LEN      len;           \
}Array_##__type

#define Array_Init(self,buf_ptr,buf_len) do{    \
    self->ptr=buf_ptr;                          \
    self->len=buf_len;                          \
}while(0)

__ARRAY_DECL(uint8_t);
__ARRAY_DECL(uint16_t);
__ARRAY_DECL(uint32_t);

__ARRAY_DECL(int8_t);
__ARRAY_DECL(int16_t);
__ARRAY_DECL(int32_t);

//Const usage
//Array_uint16_t ary = { (uint16_t[]){1,2,3,4,5}, 5 };


#endif
