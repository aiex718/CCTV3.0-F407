#ifndef __MJPEGD_SNAP_H__
#define __MJPEGD_SNAP_H__

#include "lwip/err.h"
#include "eth/apps/mjpeg/mjpegd_client.h"

err_t mjpegd_nextframe_snap_start(client_state_t* cs);
err_t mjpegd_nextframe_snap_body(client_state_t* cs);
err_t mjpegd_nextframe_snap_finish(client_state_t* cs);

#endif // __MJPEGD_SNAP_H__
