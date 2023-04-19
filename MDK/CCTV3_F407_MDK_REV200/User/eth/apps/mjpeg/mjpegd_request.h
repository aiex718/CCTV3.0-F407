#ifndef __MJPEGD_REQUEST_H__
#define __MJPEGD_REQUEST_H__

#include "eth/apps/mjpeg/mjpegd_fwdecl.h"
#include "eth/apps/mjpeg/mjpegd_client.h"

typedef enum 
{
    REQUEST_NOTFOUND = 0,
    REQUEST_TOOMANY,
    REQUEST_HANDSHAKE,
    REQUEST_VIEW_SNAP,
    REQUEST_VIEW_STREAM,
    REQUEST_SNAP,
    REQUEST_STREAM,
    
    REQUEST_MAX
} request_enum;
typedef s8_t request_t;

#if MJPEGD_DEBUG
#define mjpegd_strreq(x) (request_strreq[x])
static const char *request_strreq[] = {
    "REQUEST_NOTFOUND",
    "REQUEST_TOOMANY",
    "REQUEST_HANDSHAKE",
    "REQUEST_VIEW_SNAP",
    "REQUEST_VIEW_STREAM",
    "REQUEST_SNAP",
    "REQUEST_STREAM"
};
#else
#define mjpegd_strreq(x) ""
#endif


struct request_handler_struct
{
    const request_t req;
    const char      *url;
    const char      *response;
    const u16_t     response_len;
    const get_nextfile_func get_nextfile;
    const recv_request_func recv_request;
    const clsd_request_func clsd_request;
};



#endif // __MJPEGD_REQUEST_H__
