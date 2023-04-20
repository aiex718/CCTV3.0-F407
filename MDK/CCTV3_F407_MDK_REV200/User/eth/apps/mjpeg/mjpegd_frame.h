#ifndef MJPEGD_FRAME_H
#define MJPEGD_FRAME_H

#include "bsp/sys/semaphore.h"
#include "bsp/sys/systime.h"
#include "lwip/opt.h"

//this is the size of the frame buffer
//for 320x240 and default Qs, 10K is pretty enough
//actual available size is MJPEGD_FRAME_MEM_SPACE - MJPEGD_FRAME_HEADER_SPACE
#ifndef MJPEGD_FRAME_MEM_SPACE
    #define MJPEGD_FRAME_MEM_SPACE (10*1024) //10K
#endif
//we reserve some space for mjpeg header
#ifndef MJPEGD_FRAME_HEADER_SPACE
    #define MJPEGD_FRAME_HEADER_SPACE 128
#endif

#ifndef MJPEGD_FRAME_COMMENT_SPACE
    #define MJPEGD_FRAME_COMMENT_SPACE 16
#endif

#define MJPEGD_FRAME_PAYLOAD_SPACE (MJPEGD_FRAME_MEM_SPACE - MJPEGD_FRAME_HEADER_SPACE)
#define MJPEGD_FRAME_SEMAPHORE_MAX MJPEGD_TOTAL_CLEINT_LIMIT

typedef struct Mjpegd_Frame_struct Mjpegd_Frame_t;
struct Mjpegd_Frame_struct
{
    u8_t* head;
    u8_t* payload;
    u16_t payload_len;
    u8_t* tail;
    SysTime_t capture_time;

    //dont modify these private member
    Semaphore_t _sem;

    u8_t _mem[MJPEGD_FRAME_MEM_SPACE];
};

#define Mjpegd_Frame_HeaderSize(frame) ((frame)->payload - (frame)->head)
#define Mjpegd_Frame_HeaderAvailable(frame) ((frame)->head - frame->_mem)
#define Mjpegd_Frame_TailSize(frame) ((frame)->tail - ((frame)->payload+(frame)->payload_len))
#define Mjpegd_Frame_TailAvailable(frame) (frame->_mem + sizeof(frame->_mem) - (frame)->tail)
#define Mjpegd_Frame_TotalSize(frame) ((frame)->tail - (frame)->head)

#define Mjpegd_Frame_IsValid(frame) ((frame)!=NULL && (frame)->payload_len!=0)


void Mjpegd_Frame_Init(Mjpegd_Frame_t* self);
void Mjpegd_Frame_Clear(Mjpegd_Frame_t* self);
void Mjpegd_Frame_CaptureFinish(Mjpegd_Frame_t* self,uint16_t len);
u16_t Mjpegd_Frame_InsertComment(Mjpegd_Frame_t* self,const u8_t *data, u16_t w_len);
u16_t Mjpegd_Frame_WriteHeader(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len);
u16_t Mjpegd_Frame_WriteTail(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len);

#define Mjpegd_Frame_TryAcquire(frame) Semaphore_TryDown(&frame->_sem)
#define Mjpegd_Frame_Release(frame) Semaphore_Up(&frame->_sem);
#define Mjpegd_Frame_TryLock(frame) Semaphore_TryDownMulti(&frame->_sem,MJPEGD_FRAME_SEMAPHORE_MAX)
#define Mjpegd_Frame_Unlock(frame) Semaphore_UpMulti(&frame->_sem,MJPEGD_FRAME_SEMAPHORE_MAX)


#endif
