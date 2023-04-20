#ifndef __MJPEGD_TYPEDEF_H__
#define __MJPEGD_TYPEDEF_H__

#include "bsp/sys/systime.h"
#include "lwip/err.h"

#define Mjpegd_Systime_t SysTime_t
typedef err_t (*Mjpegd_Callback_t)(void *arg);

#endif
