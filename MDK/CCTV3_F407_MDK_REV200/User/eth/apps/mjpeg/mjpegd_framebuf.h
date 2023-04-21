#ifndef MJPEGD_FRAMEBUF_H
#define MJPEGD_FRAMEBUF_H

#include "eth/apps/mjpeg/mjpegd_frame.h"
#include "eth/apps/mjpeg/mjpegd_typedef.h"


typedef enum 
{
    //sender is frame_buf Mjpegd_FrameBuf_t*
    //arg is new frame Mjpegd_Frame_t* 
    FRAMEBUF_CALLBACK_RX_NEWFRAME =0    ,
    __NOT_CALLBACK_FRAMEBUF_MAX         ,
}Mjpegd_FrameBuf_CallbackIdx_t;

typedef struct Mjpegd_FrameBuf_struct
{
    //callback
    Mjpegd_Callback_fn FrameBuf_Callbacks[__NOT_CALLBACK_FRAMEBUF_MAX];
    //private, dont use
    Mjpegd_Frame_t *_frames;
    u8_t _frames_len;

}Mjpegd_FrameBuf_t;

void Mjpegd_FrameBuf_Init(Mjpegd_FrameBuf_t* self);
void Mjpegd_FrameBuf_SetCallback(Mjpegd_FrameBuf_t* self, Mjpegd_FrameBuf_CallbackIdx_t cb_idx, Mjpegd_Callback_fn callback_fn);

Mjpegd_Frame_t* Mjpegd_FrameBuf_GetLatest(Mjpegd_FrameBuf_t* self,Mjpegd_SysTime_t last_frame_time);
void Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf_t* self,Mjpegd_Frame_t* frame);

Mjpegd_Frame_t* Mjpegd_FrameBuf_GetIdle(Mjpegd_FrameBuf_t* self);
void Mjpegd_FrameBuf_ReleaseIdle(Mjpegd_FrameBuf_t* self,Mjpegd_Frame_t* frame);

#endif
