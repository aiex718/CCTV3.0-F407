#ifndef MJPEGD_FRAME_H
#define MJPEGD_FRAME_H


#include "app/mjpegd/mjpegd_opts.h"
#include "app/mjpegd/mjpegd_semaphore.h"

typedef struct Mjpegd_Frame_s Mjpegd_Frame_t;
struct Mjpegd_Frame_s
{
    u8_t* head;
    u8_t* payload;
    u8_t* tail;
    u8_t* eof;
    MJPEGD_SYSTIME_T capture_time;
    //payload size
    u16_t payload_len;
    //captured frame size, if 0, frame is invalid
    //if equal to payload_len, frame size may be larger than payload_len
    //and only receive partial frame, when meanning frame is INVALID 
    u16_t frame_len;

    //dont modify these private member
    u8_t _mem[MJPEGD_FRAME_MEM_SPACE];
    Mjpegd_Semaphore_t _sem;
};

#define Mjpegd_Frame_HeaderSize(frame) ((frame)->payload - (frame)->head)
#define Mjpegd_Frame_HeaderAvailable(frame) ((frame)->head - frame->_mem)
#define Mjpegd_Frame_TailSize(frame) ((frame)->tail - ((frame)->payload+(frame)->frame_len))
#define Mjpegd_Frame_TailAvailable(frame) (frame->_mem + sizeof(frame->_mem) - (frame)->tail)
#define Mjpegd_Frame_EofSize(frame) ((frame)->eof - (frame)->tail)
#define Mjpegd_Frame_EofAvailable(frame) (frame->_mem + sizeof(frame->_mem) - (frame)->eof)

#define Mjpegd_Frame_StreamSize(frame) ((frame)->tail - (frame)->head)
#define Mjpegd_Frame_SnapSize(frame) ((frame)->eof - (frame)->head)

#define Mjpegd_Frame_IsValid(frame) ((frame)!=NULL && \
    (frame)->frame_len!=0 && (frame)->frame_len!=(frame)->payload_len)
#define Mjpegd_Frame_IsClear(frame) ((frame)!=NULL && \
    (frame)->frame_len==0 && (frame)->capture_time==0)

void Mjpegd_Frame_Init(Mjpegd_Frame_t* self);
void Mjpegd_Frame_Clear(Mjpegd_Frame_t* self);
void Mjpegd_Frame_SetLenAndTime(Mjpegd_Frame_t* self,uint16_t len);
u16_t Mjpegd_Frame_InsertComment(Mjpegd_Frame_t* self,const u8_t *data, u16_t w_len);
u16_t Mjpegd_Frame_WriteHeader(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len);
u16_t Mjpegd_Frame_WriteTail(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len);
u16_t Mjpegd_Frame_WriteEOF(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len);

#define Mjpegd_Frame_TryAcquire(frame) Mjpegd_Semaphore_TryDown(&frame->_sem)
#define Mjpegd_Frame_Release(frame) Mjpegd_Semaphore_Up(&frame->_sem);
#define Mjpegd_Frame_TryLock(frame) Mjpegd_Semaphore_TryDownMulti(&frame->_sem,MJPEGD_FRAME_SEMAPHORE_MAX)
#define Mjpegd_Frame_Unlock(frame) Mjpegd_Semaphore_UpMulti(&frame->_sem,MJPEGD_FRAME_SEMAPHORE_MAX)

#endif
