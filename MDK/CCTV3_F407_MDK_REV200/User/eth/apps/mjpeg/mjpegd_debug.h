#ifndef __MJPEGD_DEBUG_H__
#define __MJPEGD_DEBUG_H__

#include "lwip/debug.h"

#ifndef MJPEGD_DEBUG
    #define MJPEGD_DEBUG LWIP_DBG_OFF
#endif

#ifndef MJPEGD_FRAMEBUF_DEBUG
    #define MJPEGD_FRAMEBUF_DEBUG LWIP_DBG_OFF
#endif

//TODO: change LF to ARG
//#define DBG_LF(x) ("%s#%d:" x,__MODULE__,__LINE__)
#define DBG_LF DBG_ARG
#define DBG_ARG(x,...) ("%s#%d:" x,__MODULE__,__LINE__,##__VA_ARGS__)
#define DBG_PRINT(x,...) LWIP_PLATFORM_DIAG((x ,##__VA_ARGS__))

#ifndef MJPEGD_DEBUG_PRINT_PAYLOAD
    #define MJPEGD_DEBUG_PRINT_PAYLOAD 0
#endif

#ifndef MJPEGD_FRAMEBUF_DEBUG_PRINT_SORT
    #define MJPEGD_FRAMEBUF_DEBUG_PRINT_SORT 0
#endif

#endif // __MJPEGD_DEBUG_H__