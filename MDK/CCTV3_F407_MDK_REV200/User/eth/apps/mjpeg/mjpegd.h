#ifndef __MJPEGD_H__
#define __MJPEGD_H__

#include "eth/apps/mjpeg/mjpegd_opts.h"
#include "eth/apps/mjpeg/mjpegd_callback.h"
#include "lwip/err.h"
#include "lwip/tcp.h"


//forward declaration
typedef struct Mjpegd_FrameBuf_struct Mjpegd_FrameBuf_t;
typedef struct Mjpegd_Camera_struct Mjpegd_Camera_t;
typedef struct ClientState_struct ClientState_t;
typedef struct Mjpegd_Frame_struct Mjpegd_Frame_t;

typedef struct {
    u16_t Port;
    Mjpegd_FrameBuf_t *FrameBuf;
    Mjpegd_Camera_t *Camera;

    //private, dont use
    struct tcp_pcb *_main_pcb;
    ClientState_t *_clients_list;
    u8_t _client_count;
    u8_t _stream_count;
    //pending frame and timer for fps,
    //these variables are shared between main thread and camera thread/ISR
    Mjpegd_Frame_t *_pending_frame;
    MJPEGD_SYSTIME_T _fps_timer;
    u16_t _fps_counter;
    u16_t _drop_counter;
    //timer for idle
    MJPEGD_SYSTIME_T _idle_timer;
    u8_t _framebuf_cleared;
    //callback
    Mjpegd_Callback_t RecvNewFrame_cb;
} Mjpegd_t;

#include "eth/apps/mjpeg/mjpegd_framebuf.h"
#include "eth/apps/mjpeg/mjpegd_frame.h"
#include "eth/apps/mjpeg/mjpegd_client.h"
#include "eth/apps/mjpeg/mjpegd_camera.h"

err_t Mjpegd_Init(Mjpegd_t *mjpegd);
void Mjpegd_Service(void *arg);

#endif // __MJPEGD_H__