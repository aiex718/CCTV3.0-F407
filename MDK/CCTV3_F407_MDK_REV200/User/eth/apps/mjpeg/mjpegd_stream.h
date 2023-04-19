#ifndef __MJPEGD_STREAM_H__
#define __MJPEGD_STREAM_H__

#include "lwip/err.h"
#include "eth/apps/mjpeg/mjpegd_client.h"

uint8_t mjpegd_stream_get_client_count(void);
void mjpegd_stream_output(void);
err_t mjpegd_nextframe_stream(client_state_t* cs);
err_t mjpegd_stream_recv_request(client_state_t* cs);
err_t mjpegd_stream_clsd_request(client_state_t* cs);

#endif // __MJPEGD_STREAM_H__
