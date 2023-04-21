#ifndef __MJPEGD_TYPEDEF_H__
#define __MJPEGD_TYPEDEF_H__

#include "lwip/err.h"

#define Mjpegd_SysTime_t u32_t
typedef err_t (*Mjpegd_Callback_fn)(void *sender,void *arg);

#endif
