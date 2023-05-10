#ifndef __MJPEGD_H__
#define __MJPEGD_H__

#include "app/mjpegd/mjpegd_opts.h"
#include "app/mjpegd/mjpegd_callback.h"
#include "lwip/err.h"

typedef struct Mjpegd_ConfigFile_s
{
    uint16_t Mjpegd_Port;
    uint8_t __padding[2];
}Mjpegd_ConfigFile_t;

typedef struct Mjpegd_s
{
    struct Mjpegd_FramePool_s *FramePool;
    struct Mjpegd_Camera_s *Camera;
    u16_t Port;
    //private, dont use
    u8_t _client_count;
    u8_t _stream_count;
    struct tcp_pcb *_main_pcb;
    struct ClientState_s *_clients_list;
    //pending frame and timer for fps,
    //these variables are shared between main thread and camera thread/ISR
    struct Mjpegd_Frame_s *_pending_frame;
    //timer for idle check
    MJPEGD_SYSTIME_T _idle_timer;
    //timer for fps check
    MJPEGD_SYSTIME_T _fps_timer;
    u16_t _fps_counter;
    u16_t _drop_counter;
    //callback
    Mjpegd_Callback_t RecvNewFrame_cb;
} Mjpegd_t;


err_t Mjpegd_Init(Mjpegd_t *mjpegd);
void Mjpegd_ConfigSet(Mjpegd_t *mjpegd,const Mjpegd_ConfigFile_t *config);
void Mjpegd_ConfigExport(const Mjpegd_t *mjpegd,Mjpegd_ConfigFile_t *config);
bool Mjpegd_IsConfigValid(Mjpegd_t *mjpegd,const Mjpegd_ConfigFile_t *config);
void Mjpegd_Service(void *arg);

#endif // __MJPEGD_H__
