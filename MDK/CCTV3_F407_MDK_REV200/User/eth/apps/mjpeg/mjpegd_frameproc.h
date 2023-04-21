#ifndef MJPEGD_FRAMEPROC_H
#define MJPEGD_FRAMEPROC_H

#include "lwip/err.h"
#include "eth/apps/mjpeg/mjpegd.h"
#include "eth/apps/mjpeg/mjpegd_framebuf.h"
#include "eth/apps/mjpeg/mjpegd_frame.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"

Mjpegd_Frame_t* Mjpegd_FrameProc_NextFrame(Mjpegd_t *mjpeg,Mjpegd_Frame_t* frame);
void Mjpegd_FrameProc_ProcPending(Mjpegd_t *mjpeg);


#endif
