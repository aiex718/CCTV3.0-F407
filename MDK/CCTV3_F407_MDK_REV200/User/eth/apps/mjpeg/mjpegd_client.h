#ifndef __MJPEGD_CLIENT_H__
#define __MJPEGD_CLIENT_H__

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/systime.h"

#include "eth/apps/mjpeg/mjpegd_fwdecl.h"
#include "eth/apps/mjpeg/mjpegd_request.h"
#include "eth/apps/mjpeg/mjpegd_frame.h"


/** Helper macro as function **/
#define client_assign_file(_cs,_file,_len) do{  \
_cs->file = (_file);                            \
_cs->file_len = (_len);                         \
_cs->file_wptr = cs->file;                      \
_cs->retries=0;                                 \
} while (0)
#define client_file_isvalid(_cs) (cs->file!=NULL && cs->file_wptr!=NULL && cs->file_len>0)
#define client_file_bytestosend(_cs) (_cs->file + _cs->file_len - _cs->file_wptr)


typedef enum 
{
    CS_NONE = 0,
    CS_ACCEPTED,
    CS_RECEIVED,
    CS_CLOSING
}conn_state_enum;
typedef s8_t conn_state_t;

struct client_state_struct 
{
    struct tcp_pcb* pcb;
    const request_handler_t* request_handler;
    conn_state_t conn_state;
    
    //buffer for dynamic content
    u8_t *buffer;

    u8_t *file;
    u8_t *file_wptr;
    u16_t file_len;

    //callback when eof reached
    get_nextfile_func get_nextfile;

    Mjpegd_Frame_t* frame;
    SysTime_t previous_frame_time;
    SysTime_t previous_transfer_time;

    u8_t retries;

    //private linklist
    client_state_t *_next;
};

client_state_t* mjpegd_get_clients(void);
u8_t mjpegd_get_client_count(void);

client_state_t* mjpegd_new_client(struct tcp_pcb *pcb);
err_t mjpegd_free_client(struct tcp_pcb *pcb, client_state_t *cs);

#endif // __MJPEGD_CLIENT_H__
