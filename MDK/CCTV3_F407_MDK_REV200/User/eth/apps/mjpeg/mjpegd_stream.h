#ifndef __MJPEGD_STREAM_H__
#define __MJPEGD_STREAM_H__


#include "lwip/err.h"
#include "eth/apps/mjpeg/mjpegd.h"

void Mjpegd_Stream_Output(Mjpegd_t *mjpegd);
err_t Mjpegd_Stream_FrameSent(struct ClientState_s *cs);
err_t Mjpegd_Stream_RecvRequest(struct ClientState_s *cs);
err_t Mjpegd_Stream_CloseRequest(struct ClientState_s *cs);

#endif // __MJPEGD_STREAM_H__
