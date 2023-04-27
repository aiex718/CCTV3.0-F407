#ifndef __MJPEGD_STREAM_H__
#define __MJPEGD_STREAM_H__


#include "lwip/err.h"
#include "eth/apps/mjpeg/mjpegd.h"

void Mjpegd_Stream_Output(Mjpegd_t *mjpegd);
err_t Mjpegd_Stream_FrameSent(ClientState_t *cs);
err_t Mjpegd_Stream_RecvRequest(ClientState_t *cs);
err_t Mjpegd_Stream_CloseRequest(ClientState_t *cs);

#endif // __MJPEGD_STREAM_H__
