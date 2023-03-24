#ifndef DBG_SERIAL_H
#define DBG_SERIAL_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/concurrent_queue.h"

#define DBG_LF(x) ("%s#%d:" x,__MODULE__,__LINE__)
#define DBG_ARG(x,...) ("%s#%d:" x,__MODULE__,__LINE__,__VA_ARGS__)




#endif
