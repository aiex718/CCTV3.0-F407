#ifndef PLATFORM_CALLBACKS_H
#define PLATFORM_CALLBACKS_H

#include "bsp/platform/platform_defs.h"

#ifndef LONGPRESS_RESET_DELAY 
    #define LONGPRESS_RESET_DELAY 3000//ms
#endif

void Platform_RegistCallbacks(void);

#endif
