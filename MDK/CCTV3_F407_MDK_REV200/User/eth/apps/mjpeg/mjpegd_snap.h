#ifndef __MJPEGD_SNAP_H__
#define __MJPEGD_SNAP_H__

#include "lwip/err.h"
#include "eth/apps/mjpeg/mjpegd_client.h"

err_t mjpegd_nextframe_snap_start(void* client_state);
err_t mjpegd_nextframe_snap_body(void* client_state);
err_t mjpegd_nextframe_snap_finish(void* client_state);

#endif // __MJPEGD_SNAP_H__
