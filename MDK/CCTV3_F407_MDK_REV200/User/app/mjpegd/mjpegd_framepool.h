#ifndef MJPEGD_FRAMEPOOL_H
#define MJPEGD_FRAMEPOOL_H

#include "app/mjpegd/mjpegd_frame.h"
#include "app/mjpegd/mjpegd_callback.h"
#include "app/mjpegd/mjpegd_opts.h"

typedef enum 
{
    //sender is frame_pool Mjpegd_FramePool_t*
    //arg is new frame Mjpegd_Frame_t* 
    FRAMEPOOL_CALLBACK_RX_NEWFRAME =0    ,
    __NOT_CALLBACK_FRAMEPOOL_MAX         ,
}Mjpegd_FramePool_CallbackIdx_t;

typedef struct Mjpegd_FramePool_s
{
    //callback
    Mjpegd_Callback_t *FramePool_Callbacks[__NOT_CALLBACK_FRAMEPOOL_MAX];
    //private, dont use
    Mjpegd_Frame_t *_frames;
    u8_t _frames_len;

}Mjpegd_FramePool_t;

void Mjpegd_FramePool_Init(Mjpegd_FramePool_t* self);
bool Mjpegd_FramePool_TryClear(Mjpegd_FramePool_t* self);
void Mjpegd_FramePool_SetCallback(
    Mjpegd_FramePool_t* self, Mjpegd_FramePool_CallbackIdx_t cb_idx, 
    Mjpegd_Callback_t *callback);

Mjpegd_Frame_t* Mjpegd_FramePool_GetLatest(
    Mjpegd_FramePool_t* self,MJPEGD_SYSTIME_T last_frame_time);
void Mjpegd_FramePool_Release(
    Mjpegd_FramePool_t* self,Mjpegd_Frame_t* frame);

Mjpegd_Frame_t* Mjpegd_FramePool_GetIdle(Mjpegd_FramePool_t* self);
void Mjpegd_FramePool_ReturnIdle(
    Mjpegd_FramePool_t* self,Mjpegd_Frame_t* frame);

#endif
