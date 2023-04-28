#ifndef __MJPEGD_OPTS_H__
#define __MJPEGD_OPTS_H__

#include "lwip/opt.h"

/*  Total stream clients limit, if exceed, new client will get a 429 error.
    Snap clients are not limited by MJPEGD_STREAM_CLIENT_LIMIT.
    
    Note: Each stream client use 2 TCP_SEG and 1 TCP_PCB,
    edit lwipopts.h to fit your needs. */
#ifndef MJPEGD_STREAM_CLIENT_LIMIT
    #define MJPEGD_STREAM_CLIENT_LIMIT 5
#endif

/*  The MJPEGD_FRAMEPOOL_LEN is the max number of frames that can be buffered.
    Total ram usage is MJPEGD_FRAMEPOOL_LEN * MJPEGD_FRAME_MEM_SPACE.
    Each frame is not exclusive to one client, it's shared by multiple clients.

    Depending on your clients network speed and packet loss rate,
    if all clients are fast, you can set a smaller buffer length,
    better not less than 3.
    
    The most secure buffer length is MJPEGD_STREAM_CLIENT_LIMIT+2.
    Each client can have 1 buffer frame at worst case,
    1 pending frame waiting to be processed by mjpegd_service,
    and 1 frame is being captured by camera.   */
#ifndef MJPEGD_FRAMEPOOL_LEN
    #define MJPEGD_FRAMEPOOL_LEN (MJPEGD_STREAM_CLIENT_LIMIT+2)
#endif

/* Total client limit, including stream and snap clients */
#ifndef MJPEGD_TOTAL_CLIENT_LIMIT
    #define MJPEGD_TOTAL_CLIENT_LIMIT MJPEGD_STREAM_CLIENT_LIMIT+3
#endif

/*  this is the size of each frame
    for 320x240 and default Qs, 12K is pretty enough
    actual available size is MJPEGD_FRAME_MEM_SPACE - MJPEGD_FRAME_HEADER_SPACE */
#ifndef MJPEGD_FRAME_MEM_SPACE
    #define MJPEGD_FRAME_MEM_SPACE (12*1024) //12K
#endif

/* we reserve some space for mjpeg header */
#ifndef MJPEGD_FRAME_HEADER_SPACE
    #define MJPEGD_FRAME_HEADER_SPACE 128
#endif

#ifndef MJPEGD_FRAME_COMMENT_SPACE
    #define MJPEGD_FRAME_COMMENT_SPACE 16
#endif

#ifndef MJPEGD_SHOWDROP_THRESHOLD
    #define MJPEGD_SHOWDROP_THRESHOLD 10
#endif

#ifndef MJPEGD_ALLOW_STREAM_CORS
    #define MJPEGD_ALLOW_STREAM_CORS 0
#endif

#ifndef MJPEGD_SHOWFPS_PERIOD
    #define MJPEGD_SHOWFPS_PERIOD 0 //(2^n) seconds
#endif

#ifndef MJPEGD_SERVICE_PERIOD
    #define MJPEGD_SERVICE_PERIOD 10//10ms
#endif

#ifndef MJPEGD_MAX_URL_PARAMETERS 
    #define MJPEGD_MAX_URL_PARAMETERS 6
#endif

#ifndef MJPEGD_SYSTIME_T
    #define MJPEGD_SYSTIME_T u32_t
#endif

#ifndef MJPEGD_GET_UNIX_TIMESTAMP
    #define MJPEGD_GET_UNIX_TIMESTAMP 0
#endif

//poll interval is n*2*TCP_TMR_INTERVAL(typically 250ms)
#ifndef MJPEGD_POLL_INTERVAL
    #define MJPEGD_POLL_INTERVAL 2 //2*2*250=1000ms
#endif

//retry limit is n*(poll interval)
#ifndef MJPEGD_MAX_RETRIES
    #define MJPEGD_MAX_RETRIES 8 //8x1000ms=8s
#endif

#ifndef MJPEGD_MAIN_TCP_PRIO
    #define MJPEGD_MAIN_TCP_PRIO TCP_PRIO_NORMAL
#endif

#ifndef MJPEGD_STREAM_TCP_PRIO
    #define MJPEGD_STREAM_TCP_PRIO TCP_PRIO_MIN
#endif

#ifndef MJPEGD_MIN_REQ_LEN
    #define MJPEGD_MIN_REQ_LEN 7
#endif

#ifndef MJPEGD_IDLE_TIMEOUT
    #define MJPEGD_IDLE_TIMEOUT 30000 //30s
#endif

#endif
