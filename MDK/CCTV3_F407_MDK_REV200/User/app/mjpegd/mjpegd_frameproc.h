#ifndef MJPEGD_FRAMEPROC_H
#define MJPEGD_FRAMEPROC_H

#include "lwip/err.h"
#include "app/mjpegd/mjpegd.h"
#include "app/mjpegd/mjpegd_framepool.h"
#include "app/mjpegd/mjpegd_frame.h"
#include "app/mjpegd/mjpegd_memutils.h"

void Mjpegd_FrameProc_RecvBroken(Mjpegd_t *mjpegd,Mjpegd_Frame_t* frame);
void Mjpegd_FrameProc_RecvRaw(Mjpegd_t *mjpegd,Mjpegd_Frame_t* frame);
Mjpegd_Frame_t* Mjpegd_FrameProc_NextFrame(Mjpegd_t *mjpegd,Mjpegd_Frame_t* frame);
void Mjpegd_FrameProc_ProcPending(Mjpegd_t *mjpegd);


#endif
