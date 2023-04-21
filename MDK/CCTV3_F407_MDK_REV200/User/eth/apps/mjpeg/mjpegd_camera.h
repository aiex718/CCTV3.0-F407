#ifndef MJPEGD_CAMERA_H
#define MJPEGD_CAMERA_H

#include "lwip/arch.h"
#include "eth/apps/mjpeg/mjpegd_frame.h"

typedef struct Mjpegd_Camera_struct
{
    int a;
    //cb....
}Mjpegd_Camera_t;

void Mjpegd_Camera_Init(Mjpegd_Camera_t *cam);
void Mjpegd_Camera_Start(Mjpegd_Camera_t *cam);
void Mjpegd_Camera_DoSnap(Mjpegd_Camera_t *cam,Mjpegd_Frame_t *frame_buf);
void Mjpegd_Camera_Stop(Mjpegd_Camera_t *cam);

u8_t Mjpegd_Camera_IsStarted(Mjpegd_Camera_t *cam);
u8_t Mjpegd_Camera_IsSnapping(Mjpegd_Camera_t *cam);

#endif
