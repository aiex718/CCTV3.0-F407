#include "eth/apps/mjpeg/mjpegd_frame.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"

void Mjpegd_Frame_Init(Mjpegd_Frame_t* self)
{
    Mjpegd_Frame_Clear(self);
    Semaphore_Init(&self->_sem,MJPEGD_FRAME_SEMAPHORE_MAX);
}

void Mjpegd_Frame_Clear(Mjpegd_Frame_t* self)
{
    self->payload = self->_mem+MJPEGD_FRAME_HEADER_SPACE;
    self->head = self->payload;
    self->tail = self->payload;
    self->payload_len=0;
    self->capture_time=0;
}

void Mjpegd_Frame_CaptureFinish(Mjpegd_Frame_t* self,uint16_t len)
{
    self->capture_time = SysTime_Get();
    self->payload_len = len;
    self->tail = self->payload+self->payload_len;
}

/**
 * @brief insert comment into frame 
 * @warning this function must be called before Mjpegd_Frame_WriteHeader
 *          if header has been written, no comment allowed
 * @param self frame object
 * @param data comment data
 * @param w_len comment data length
 * @return actual written length
 */
u16_t Mjpegd_Frame_InsertComment(Mjpegd_Frame_t* self,const u8_t *data, u16_t w_len)
{
    static const u8_t Jpeg_Comment_Section[6+MJPEGD_FRAME_COMMENT_SPACE] =
    {
        //jpeg SOI
        0xFF,0xD8,
        //jpeg COM section tag
        0xFF,0xFE,
        //comment len indicator
        ((MJPEGD_FRAME_COMMENT_SPACE+2)>>8)&0xff,
        (MJPEGD_FRAME_COMMENT_SPACE+2)&0xff,
    };

    if( self->payload==self->head &&
        Mjpegd_Frame_HeaderAvailable(self)>sizeof(Jpeg_Comment_Section))
    {
        u8_t* comment_wptr;
        w_len = MJPEGD_MIN(w_len,MJPEGD_FRAME_COMMENT_SPACE);
        //make space for comment
        self->payload -= sizeof(Jpeg_Comment_Section)-2;
        self->head -= sizeof(Jpeg_Comment_Section)-2;
        self->payload_len += sizeof(Jpeg_Comment_Section)-2;
        
        //write comment block
        MJPEGD_MEMCPY(self->payload,Jpeg_Comment_Section,sizeof(Jpeg_Comment_Section));
        comment_wptr = self->head+6;

        //write comment content
        MJPEGD_MEMCPY(comment_wptr,data,w_len);
        return w_len;
    }

    return 0;
}

u16_t Mjpegd_Frame_WriteHeader(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len)
{
    if(w_len<Mjpegd_Frame_HeaderAvailable(self))
    {
        self->head-=w_len;
        MJPEGD_MEMCPY(self->head,data,w_len);
        return w_len;
    }
    else
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("WriteHeader out of space %d<%d\n",w_len,Mjpegd_Frame_HeaderAvailable(self)));
    }
    
    return 0;
}

u16_t Mjpegd_Frame_WriteTail(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len)
{
    if (w_len < Mjpegd_Frame_TailAvailable(self))
    {
        MJPEGD_MEMCPY(self->tail,data,w_len);
        self->tail+=w_len;
        return w_len;
    }
    else
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("WriteTail out of space %d<%d\n",w_len,Mjpegd_Frame_TailAvailable(self)));
    }
    return 0;
}
