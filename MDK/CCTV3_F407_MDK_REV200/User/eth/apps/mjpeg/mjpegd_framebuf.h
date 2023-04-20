#ifndef MJPEGD_FRAMEBUF_H
#define MJPEGD_FRAMEBUF_H

#include "eth/apps/mjpeg/mjpegd_frame.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/callback.h"


typedef enum 
{
    //all callbacks are invoked in Mjpegd_FrameBuf_Service
    //sender is frame_buf Mjpegd_FrameBuf_t*
    //arg is new frame Mjpegd_Frame_t* 
    FRAMEBUF_CALLBACK_RX_RAWFRAME = 0   ,
    FRAMEBUF_CALLBACK_RX_NEWFRAME       ,
    __NOT_CALLBACK_FRAMEBUF_MAX         ,
}Mjpegd_FrameBuf_CallbackIdx_t;

typedef struct 
{
    //callback
    Callback_t* Mjpegd_FrameBuf_Callbacks[__NOT_CALLBACK_FRAMEBUF_MAX];
    //private, dont use
    Mjpegd_Frame_t *_frames;
    u8_t _frames_len;
    Mjpegd_Frame_t *_pending_frame;
    SysTime_t _fps_timer;
    u16_t _fps_counter;
}Mjpegd_FrameBuf_t;

//TODO: find a location to put Mjpegd_FrameBuf instance
extern Mjpegd_FrameBuf_t* Mjpegd_FrameBuf;

void Mjpegd_FrameBuf_Init(Mjpegd_FrameBuf_t* self);
void Mjpegd_FrameBuf_SetCallback(Mjpegd_FrameBuf_t* self, Mjpegd_FrameBuf_CallbackIdx_t cb_idx, Callback_t* callback);
void Mjpegd_FrameBuf_Service(Mjpegd_FrameBuf_t* self);

Mjpegd_Frame_t* Mjpegd_FrameBuf_GetLatest(Mjpegd_FrameBuf_t* self,SysTime_t frame_time);
void Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf_t* self,Mjpegd_Frame_t* frame);

Mjpegd_Frame_t* Mjpegd_FrameBuf_GetIdle(Mjpegd_FrameBuf_t* self);
void Mjpegd_FrameBuf_ReturnIdle(Mjpegd_FrameBuf_t* self,Mjpegd_Frame_t* frame);

#endif
