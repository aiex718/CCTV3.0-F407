#include "eth/apps/mjpeg/mjpegd_frame.h"
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

uint16_t Mjpegd_Frame_WriteComment(Mjpegd_Frame_t* self,const uint8_t *data, uint16_t len)
{
    const static uint8_t Jpeg_Comment_Tag[]={0xFF,0xFE};
    uint8_t* wptr = BSP_MEMSEARCH(self->payload,self->payload_len,
                            Jpeg_Comment_Tag,BSP_ARR_LEN(Jpeg_Comment_Tag),false);
    uint16_t wlen=0;
    if(wptr!=NULL)
    {   
        //find next 0xff, calculate the available length for comment
        uint8_t* wptr_end;
        wptr+=BSP_ARR_LEN(Jpeg_Comment_Tag);
        wptr_end = wptr;
        while(wptr_end<self->payload+self->payload_len)
        {
            if(*wptr_end==0xFF)
            {
                wlen = BSP_MIN(wptr_end - wptr,len);
                BSP_MEMCPY(wptr,data,wlen);
                break;
            }
        }
    }
    return wlen;
}

uint16_t Mjpegd_Frame_WriteHeader(Mjpegd_Frame_t* self, const uint8_t *data, uint16_t w_len)
{
    if(w_len<Mjpegd_Frame_HeaderAvailable(self))
    {
        self->head-=w_len;
        BSP_MEMCPY(self->head,data,w_len);
        return w_len;
    }
    else
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("WriteHeader out of space %d<%d\n",w_len,Mjpegd_Frame_HeaderAvailable(self)));
    }
    
    return 0;
}

uint16_t Mjpegd_Frame_WriteTail(Mjpegd_Frame_t* self, const uint8_t *data, uint16_t w_len)
{
    if (w_len < Mjpegd_Frame_TailAvailable(self))
    {
        BSP_MEMCPY(self->tail,data,w_len);
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