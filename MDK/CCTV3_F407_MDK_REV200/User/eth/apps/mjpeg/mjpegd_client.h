#ifndef __MJPEGD_CLIENT_H__
#define __MJPEGD_CLIENT_H__

#include "eth/apps/mjpeg/mjpegd.h"
#include "eth/apps/mjpeg/mjpegd_opts.h"
#include "eth/apps/mjpeg/mjpegd_request.h"
#include "eth/apps/mjpeg/mjpegd_frame.h"


/** Helper macro as function **/
#define client_assign_file(_cs,_file,_len) do{  \
(_cs)->file = (_file);                          \
(_cs)->file_len = (_len);                       \
(_cs)->file_wptr = cs->file;                    \
(_cs)->retries=0;                               \
} while (0)
#define client_file_isvalid(_cs) ((_cs)->file!=NULL && (_cs)->file_wptr!=NULL && (_cs)->file_len>0)
#define client_file_bytestosend(_cs) ((_cs)->file + (_cs)->file_len - (_cs)->file_wptr)


typedef enum 
{
    CS_NONE = 0,
    CS_ACCEPTED,
    CS_RECEIVED,
    CS_CLOSING
}conn_state_enum;
typedef s8_t conn_state_t;

typedef struct ClientState_struct ClientState_t;
struct ClientState_struct 
{
    struct tcp_pcb* pcb;
    void* parent_mjpeg;

    const Mjpegd_RequestHandler_t* request_handler;
    conn_state_t conn_state;

    //file to send
    u8_t *file;
    u8_t *file_wptr;
    u16_t file_len;

    //callback when eof reached to get next file
    //if NULL, client will be closed
    err_t (*get_nextfile_func)(void* client_state);
    
    Mjpegd_Frame_t* frame;
    MJPEGD_SYSTIME_T previous_frame_time;
    u8_t retries;

    //private linklist
    ClientState_t *_next;
};

ClientState_t* Mjpegd_Client_New(Mjpegd_t *mjpegd, struct tcp_pcb *pcb);
void Mjpegd_Client_Free(ClientState_t *cs);

err_t Mjpegd_Client_ParseRequest(ClientState_t *cs,const char* req,const u16_t req_len);
err_t Mjpegd_Client_BuildResponse(ClientState_t *cs);

#endif // __MJPEGD_CLIENT_H__
