#ifndef __MJPEGD_REQUEST_H__
#define __MJPEGD_REQUEST_H__

#include "lwip/err.h"

typedef enum 
{
    REQUEST_NOTFOUND = 0,
    REQUEST_TOOMANY,
    REQUEST_HANDSHAKE,
    REQUEST_VIEW_SNAP,
    REQUEST_VIEW_STREAM,
    REQUEST_VIEW_FPS,
    REQUEST_SNAP,
    REQUEST_STREAM,
    
    __NOT_REQUEST_MAX
} request_enum;
typedef s8_t request_t;

#if MJPEGD_DEBUG
#define mjpegd_strreq(x) (request_strreq[x])
static const char *request_strreq[__NOT_REQUEST_MAX] = {
    "REQUEST_NOTFOUND",
    "REQUEST_TOOMANY",
    "REQUEST_HANDSHAKE",
    "REQUEST_VIEW_SNAP",
    "REQUEST_VIEW_STREAM",
    "REQUEST_VIEW_FPS",
    "REQUEST_SNAP",
    "REQUEST_STREAM",
};
#else
#define mjpegd_strreq(x) ""
#endif

typedef struct Mjpegd_RequestHandler_struct
{
    const request_t req;
    const char      *url;
    const char      *response;
    const u16_t     response_len;

    err_t (* const get_nextfile_func)(void *client_state);
    err_t (* const recv_request_func)(void *client_state);
    err_t (* const clsd_request_func)(void *client_state);
}Mjpegd_RequestHandler_t;

extern const Mjpegd_RequestHandler_t mjpegd_request_handlers[__NOT_REQUEST_MAX];


#endif // __MJPEGD_REQUEST_H__
