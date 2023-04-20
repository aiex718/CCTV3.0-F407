#ifndef __MJPEGD_STREAM_H__
#define __MJPEGD_STREAM_H__

#include "lwip/err.h"
#include "eth/apps/mjpeg/mjpegd_client.h"

u8_t mjpegd_stream_get_client_count(void);
void mjpegd_stream_output(void);
err_t mjpegd_nextframe_stream(void* client_state);
err_t mjpegd_stream_recv_request(void* client_state);
err_t mjpegd_stream_clsd_request(void* client_state);

#endif // __MJPEGD_STREAM_H__
