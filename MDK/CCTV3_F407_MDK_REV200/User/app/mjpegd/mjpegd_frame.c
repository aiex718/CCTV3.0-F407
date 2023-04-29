#include "app/mjpegd/mjpegd_frame.h"
#include "app/mjpegd/mjpegd_memutils.h"
#include "app/mjpegd/mjpegd_debug.h"
#include "app/mjpegd/trycatch.h"

void Mjpegd_Frame_Init(Mjpegd_Frame_t* self)
{
    Mjpegd_Frame_Clear(self);
    Semaphore_Init(&self->_sem,MJPEGD_FRAME_SEMAPHORE_MAX);
}

void Mjpegd_Frame_Clear(Mjpegd_Frame_t* self)
{
    self->payload = self->_mem+MJPEGD_FRAME_HEADER_SPACE;
    self->payload_len = sizeof(self->_mem)-MJPEGD_FRAME_HEADER_SPACE;
    self->head = self->payload;
    self->tail = self->payload;
    self->eof = self->payload;
    self->frame_len=0;
    self->capture_time=0;
}

void Mjpegd_Frame_SetLenAndTime(Mjpegd_Frame_t* self,uint16_t len)
{
    self->capture_time = sys_now();
    self->frame_len = len;
    self->tail = self->payload+self->frame_len;
    self->eof = self->tail;
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
    try
    {
        u8_t Jpeg_Comment_Section[6] = {
            0xFF,0xD8,0xFF,0xFE,(w_len>>8)&0xff,w_len&0xff
        };
        u16_t comment_len = sizeof(Jpeg_Comment_Section)+w_len;

        throwif(self->payload!=self->head,HEADER_EXIST);
        throwif(w_len==0,NO_DATA);
        throwif(comment_len >= 
            Mjpegd_Frame_HeaderAvailable(self)-2,OUTOFSPACE);

        //make space for comment
        self->payload -= comment_len-2;
        self->head = self->payload;
        self->frame_len += comment_len-2;
        
        //write comment block
        MJPEGD_MEMCPY(self->payload,Jpeg_Comment_Section,sizeof(Jpeg_Comment_Section));

        //write comment content
        MJPEGD_MEMCPY(self->payload+6,data,w_len);
    }
    catch(HEADER_EXIST)
    {
        w_len=0;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("InsertComment HEADER_EXIST\n"));
    }
    catch(NO_DATA)
    {
        w_len=0;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("InsertComment NO_DATA\n"));
    }
    catch(OUTOFSPACE)
    {
        w_len=0;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("InsertComment OUTOFSPACE %d>=%d\n",
                w_len,Mjpegd_Frame_HeaderAvailable(self)));
    }
    finally
    {
        return w_len;   
    }
}

u16_t Mjpegd_Frame_WriteHeader(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len)
{
    if(w_len>=Mjpegd_Frame_HeaderAvailable(self))
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("WriteHeader out of space %d>=%d\n",w_len,Mjpegd_Frame_HeaderAvailable(self)));
    }
    else
    {
        self->head-=w_len;
        MJPEGD_MEMCPY(self->head,data,w_len);
        return w_len;
    }
    
    return 0;
}

/**
 * @brief write tail to frame 
 * @warning this function must be called before Mjpegd_Frame_WriteEOF
 *          if EOF has been written, write tail not allowed
 * @param self frame object
 * @param data tail data
 * @param w_len tail data length
 * @return actual written length
 */
u16_t Mjpegd_Frame_WriteTail(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len)
{
    try
    {
        throwif(self->eof!=self->tail,EOF_EXIST);
        throwif(w_len>=Mjpegd_Frame_TailAvailable(self),OUTOFSPACE);
        
        MJPEGD_MEMCPY(self->tail,data,w_len);
        self->tail+=w_len;
        self->eof=self->tail;
        
    }
    catch(EOF_EXIST)
    {
        w_len=0;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("EOF exist\n"));
    }
    catch(OUTOFSPACE)
    {
        w_len=0;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("WriteTail out of space %d>=%d\n",w_len,Mjpegd_Frame_TailAvailable(self)));
    }
    finally
    {
        return w_len;
    }
}

u16_t Mjpegd_Frame_WriteEOF(Mjpegd_Frame_t* self, const u8_t *data, u16_t w_len)
{
    if(w_len>=Mjpegd_Frame_EofAvailable(self))
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("WriteEOF out of space %d>=%d\n",w_len,Mjpegd_Frame_EofAvailable(self)));
    }
    else
    {
        MJPEGD_MEMCPY(self->eof,data,w_len);
        self->eof+=w_len;
        return w_len;
    }

    return 0;
}
