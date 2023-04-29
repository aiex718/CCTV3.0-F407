#ifndef __MJPEGD_REQUEST_H__
#define __MJPEGD_REQUEST_H__

#include "lwip/err.h"
#include "app/mjpegd/mjpegd_client.h"

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

typedef struct Mjpegd_RequestHandler_s
{
    request_t req;
    const char      *url;
    const char      *response;
    u16_t     response_len;

    err_t (* get_nextfile_func)(struct ClientState_s *cs);
    err_t (* recv_request_func)(struct ClientState_s *cs);
    err_t (* clsd_request_func)(struct ClientState_s *cs);
}Mjpegd_RequestHandler_t;

extern const Mjpegd_RequestHandler_t mjpegd_request_handlers[__NOT_REQUEST_MAX];

err_t Mjpegd_Request_Parse(struct ClientState_s *cs,char* req,u16_t req_len);
err_t Mjpegd_Request_BuildResponse(struct ClientState_s *cs);

#endif // __MJPEGD_REQUEST_H__
