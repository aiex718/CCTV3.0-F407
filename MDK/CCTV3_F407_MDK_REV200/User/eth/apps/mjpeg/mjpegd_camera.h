#ifndef MJPEGD_CAMERA_H
#define MJPEGD_CAMERA_H

#include "eth/apps/mjpeg/mjpegd.h"
#include "eth/apps/mjpeg/mjpegd_frame.h"
#include "lwip/arch.h"

#include "bsp/sys/callback.h"
#include "device/cam_ov2640/cam_ov2640.h"


typedef struct Mjpegd_Camera_struct
{
    Device_CamOV2640_t *HwCam_Ov2640;
    Callback_t Ov2640_RecvRawFrame_cb;
}Mjpegd_Camera_t;

void Mjpegd_Camera_Init(Mjpegd_Camera_t *cam,Mjpegd_t *mjpegd);
bool Mjpegd_Camera_DoSnap(Mjpegd_Camera_t *cam,Mjpegd_Frame_t *frame);
void Mjpegd_Camera_Start(Mjpegd_Camera_t *cam);
void Mjpegd_Camera_Stop(Mjpegd_Camera_t *cam);

u8_t Mjpegd_Camera_IsEnabled(Mjpegd_Camera_t *cam);
u8_t Mjpegd_Camera_IsSnapping(Mjpegd_Camera_t *cam);

#endif
