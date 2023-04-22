#ifndef __MJPEGD_STREAM_H__
#define __MJPEGD_STREAM_H__


#include "lwip/err.h"
#include "eth/apps/mjpeg/mjpegd.h"

void Mjpegd_Stream_Output(Mjpegd_t *mjpegd);
err_t Mjpegd_Stream_NextFrame(void* client_state);
err_t Mjpegd_Stream_RecvRequest(void* client_state);
err_t Mjpegd_Stream_CloseRequest(void* client_state);

#endif // __MJPEGD_STREAM_H__
