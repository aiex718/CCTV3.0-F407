#ifndef __MJPEGD_DEBUG_H__
#define __MJPEGD_DEBUG_H__

#include "lwip/debug.h"

#ifndef MJPEGD_DEBUG
    #define MJPEGD_DEBUG LWIP_DBG_OFF
#endif

#define MJPEGD_DBG_ARG(x,...) ("%s#%d:" x,__MODULE__,__LINE__,##__VA_ARGS__)
#define MJPEGD_DBG_PRINT(x,...) LWIP_PLATFORM_DIAG(x ,##__VA_ARGS__)

#ifndef MJPEGD_DEBUG_PRINT_PAYLOAD
    #define MJPEGD_DEBUG_PRINT_PAYLOAD 0
#endif

#ifndef MJPEGD_FRAMEPOOL_DEBUG
    #define MJPEGD_FRAMEPOOL_DEBUG LWIP_DBG_OFF
#endif

#ifndef MJPEGD_FRAMEPOOL_DEBUG_PRINT_SORT
    #define MJPEGD_FRAMEPOOL_DEBUG_PRINT_SORT 0
#endif

#endif // __MJPEGD_DEBUG_H__
