#ifndef __MJPEGD_H__
#define __MJPEGD_H__

#include "eth/apps/mjpeg/mjpegd_typedef.h"
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
    //pending frame and timer for fps
    Mjpegd_Frame_t *_pending_frame;
    Mjpegd_SysTime_t _fps_timer;
    u16_t _fps_counter;
    u16_t _drop_counter;
} Mjpegd_t;

#include "eth/apps/mjpeg/mjpegd_framebuf.h"
#include "eth/apps/mjpeg/mjpegd_frame.h"
#include "eth/apps/mjpeg/mjpegd_client.h"
#include "eth/apps/mjpeg/mjpegd_camera.h"

err_t Mjpegd_Init(Mjpegd_t *mjpeg);
void Mjpegd_Service(void *arg);

extern Mjpegd_t* mjpeg_inst;

#endif // __MJPEGD_H__
