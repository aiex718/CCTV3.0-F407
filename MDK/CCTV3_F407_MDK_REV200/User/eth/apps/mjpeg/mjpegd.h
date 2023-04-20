#ifndef __MJPEGD_H__
#define __MJPEGD_H__

#include "lwip/err.h"

err_t mjpegd_init(u16_t port);
void mjpegd_service(void* arg);

#endif // __MJPEGD_H__
