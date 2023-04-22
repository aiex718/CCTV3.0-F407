#ifndef __MJPEGD_OPTS_H__
#define __MJPEGD_OPTS_H__

#include "lwip/opt.h"

#ifndef MJPEGD_STREAM_CLIENT_LIMIT
    #define MJPEGD_STREAM_CLIENT_LIMIT 3
#endif

//this is the size of each frame
//for 320x240 and default Qs, 12K is pretty enough
//actual available size is MJPEGD_FRAME_MEM_SPACE - MJPEGD_FRAME_HEADER_SPACE
#ifndef MJPEGD_FRAME_MEM_SPACE
    #define MJPEGD_FRAME_MEM_SPACE (12*1024) //10K
#endif

//we reserve some space for mjpeg header
#ifndef MJPEGD_FRAME_HEADER_SPACE
    #define MJPEGD_FRAME_HEADER_SPACE 128
#endif

#ifndef MJPEGD_FRAME_COMMENT_SPACE
    #define MJPEGD_FRAME_COMMENT_SPACE 16
#endif

#ifndef MJPEGD_FRAMEDROP_WARNING_THRESHOLD
    #define MJPEGD_FRAMEDROP_WARNING_THRESHOLD 10
#endif

#ifndef MJPEGD_FPS_PERIOD
    #define MJPEGD_FPS_PERIOD 3 //(2^n) seconds
#endif

#ifndef MJPEGD_SERVICE_PERIOD
    #define MJPEGD_SERVICE_PERIOD 10//10ms
#endif

#ifndef MJPEGD_SYSTIME_T
    #define MJPEGD_SYSTIME_T u32_t
#endif
//poll interval is n*2*TCP_TMR_INTERVAL
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

#define MJPEGD_FRAME_PAYLOAD_SPACE (MJPEGD_FRAME_MEM_SPACE - MJPEGD_FRAME_HEADER_SPACE)
#define MJPEGD_FRAME_SEMAPHORE_MAX MJPEGD_STREAM_CLIENT_LIMIT
#define MJPEGD_FRAMEBUF_LEN (MJPEGD_STREAM_CLIENT_LIMIT)

#endif
