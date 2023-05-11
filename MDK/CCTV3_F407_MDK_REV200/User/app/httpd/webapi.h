#ifndef WEBAPI_H
#define WEBAPI_H

#include "bsp/platform/platform_defs.h"

#ifndef WEBAPI_RESPONSE_BUFFER_LEN
    #define WEBAPI_RESPONSE_BUFFER_LEN 256
#endif

#ifndef WEBAPI_REBOOT_DELAY
    #define WEBAPI_REBOOT_DELAY 3000
#endif

extern const char* const Webapi_Enter_Point;

#endif
