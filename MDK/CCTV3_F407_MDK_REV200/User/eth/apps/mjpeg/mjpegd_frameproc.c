#include "eth/apps/mjpeg/mjpegd_frameproc.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/mjpegd_opts.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/trycatch.h"



//private functions
static void Mjpegd_FrameProc_ProcessRawFrame(Mjpegd_Frame_t* frame);

/**
 * @brief Return any broken frame.
 * @note  This function is thread safe.
 * @details Clear frame and return to frame buffer.
 * @param frame broken frame to return.
 */
void Mjpegd_FrameProc_RecvBroken(Mjpegd_t *mjpegd,Mjpegd_Frame_t* frame)
{
    if(frame!=NULL)
    {
        Mjpegd_Frame_Clear(frame);
        Mjpegd_FrameBuf_ReleaseIdle(mjpegd->FrameBuf,frame);
    }
}

/**
 * @brief Return new frame captured by camera.
 * @note  This function is thread safe.
 * @details If previous frame is not yet processed, we drop previous frame.
 * @param frame New jpeg frame captured by camera.
 */
void Mjpegd_FrameProc_RecvRaw(Mjpegd_t *mjpegd,Mjpegd_Frame_t* frame)
{
    if (Mjpegd_Frame_IsValid(frame)==false)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("RecvRaw bad frame %p\n",frame));
    }
    else
    {
        frame = (Mjpegd_Frame_t*)
            MJPEGD_ATOMIC_XCHG((__IO u32_t *)&mjpegd->_pending_frame,(u32_t)frame);
    }

    if(frame!=NULL)
    {
        MJPEGD_ATOMIC_INC(&mjpegd->_drop_counter);
        Mjpegd_Frame_Clear(frame);
        Mjpegd_FrameBuf_ReleaseIdle(mjpegd->FrameBuf,frame);
    }
}

/**
 * @brief Return new frame captured by camera, return next frame buffer
 *        for next capture if available.
 * @note  This function is thread safe.
 * @details If previous frame is not yet processed, we drop previous frame 
 *          and reuse the frame buffer for next capture.          
 *          Otherwise, get a new frame buffer for next capture.
 *          If no frame buffer available, return null.
 * @param frame New jpeg frame captured by camera.
 * @return Next frame buffer for next capture, null if not available.
 */
Mjpegd_Frame_t* Mjpegd_FrameProc_NextFrame(Mjpegd_t *mjpegd,Mjpegd_Frame_t* frame)
{
    Mjpegd_Frame_t *next_frame=NULL;

    if (Mjpegd_Frame_IsValid(frame)==false)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("NextFrame bad frame %p\n",frame));
        next_frame = frame; //reuse the frame buffer
    }
    else
    {
        next_frame = (Mjpegd_Frame_t*)
            MJPEGD_ATOMIC_XCHG((__IO u32_t *)&mjpegd->_pending_frame,(u32_t)frame);
    }

    if(next_frame!=NULL)
    {
        MJPEGD_ATOMIC_INC(&mjpegd->_drop_counter);
        Mjpegd_Frame_Clear(next_frame);
    }
    else
    {
        next_frame=Mjpegd_FrameBuf_GetIdle(mjpegd->FrameBuf);
    }

    return next_frame;
}

/**
 * @brief Take pending frame to local and process it.
 * @note  This function is thread safe.
 */
void Mjpegd_FrameProc_ProcPending(Mjpegd_t *mjpegd)
{
    Mjpegd_Frame_t* local_frame = (Mjpegd_Frame_t*)
            MJPEGD_ATOMIC_XCHG((__IO u32_t *)&mjpegd->_pending_frame,NULL);
    
    if(local_frame!=NULL)
    {
        Mjpegd_FrameProc_ProcessRawFrame(local_frame);
        Mjpegd_FrameBuf_ReleaseIdle(mjpegd->FrameBuf,local_frame);

        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_TRACE, 
            MJPEGD_DBG_ARG("ProcPending %p, _sem=%d\n",local_frame,local_frame->_sem));
        
        MJPEGD_ATOMIC_INC(&mjpegd->_fps_counter);
    }
}

/**
 * @brief Add headers, comments(contains frame time information) and HTTP EOF to received frame.
 * @note  This function is thread safe.
 * @param frame Received frame
 */
static void Mjpegd_FrameProc_ProcessRawFrame(Mjpegd_Frame_t* frame)
{
    static const char Http_Mjpeg_ContentLength[]="Content-Length: %5d   \r\n\r\n";
    static const char Http_Mjpeg_Boundary[]="--myboundary";
    static const char Http_Mjpeg_ContentType[]="Content-Type: image/jpg \r\n";
    static const char Http_CLRF[]="\r\n";
    static const char Http_ChunkedEOF[]="0\r\n\r\n";
    u8_t buf[30];
    u16_t w_len;
    
    try
    {
        throwif(frame==NULL,NULL_FRAME);
        throwif(!Mjpegd_Frame_IsValid(frame),BAD_FRAME);
        throwif(frame->head!=frame->payload,ALREADY_PROCESSED);
        //insert jpeg comment section
        //this will overwrite old frame SOI(FF D8) from frame->head
        {
            //TODO:insert RTC time 8 byte
            u8_t FakeRTC[8]={0,1,2,3,4,5,6,7};
            u8_t comment[sizeof(FakeRTC)+sizeof(frame->capture_time)]; //8 byte RTC time, 4 byte frame time
            u8_t* comment_wptr = comment;
            
            MJPEGD_MEMCPY(comment_wptr,FakeRTC,sizeof(FakeRTC));
            comment_wptr+=sizeof(FakeRTC);

            //insert frame time 4byte
            MJPEGD_MEMCPY(comment_wptr,&frame->capture_time,sizeof(frame->capture_time));
            comment_wptr+=sizeof(frame->capture_time);
            Mjpegd_Frame_InsertComment(frame,comment,sizeof(comment));
        }

        //write mjpeg header
        w_len = sprintf((char*)buf,Http_Mjpeg_ContentLength,frame->frame_len);
        Mjpegd_Frame_WriteHeader(frame,buf,w_len);
        Mjpegd_Frame_WriteHeader(frame,(u8_t*)Http_Mjpeg_ContentType,MJPEGD_CHRARR_STRLEN(Http_Mjpeg_ContentType));
        Mjpegd_Frame_WriteHeader(frame,(u8_t*)Http_Mjpeg_Boundary,MJPEGD_CHRARR_STRLEN(Http_Mjpeg_Boundary));
        w_len = sprintf((char*)buf,"%x\r\n", Mjpegd_Frame_HeaderSize(frame) + frame->frame_len);
        Mjpegd_Frame_WriteHeader(frame,buf,w_len);

        //write mjpegd tail, chunked block end
        Mjpegd_Frame_WriteTail(frame,(u8_t*)Http_CLRF,MJPEGD_CHRARR_STRLEN(Http_CLRF));
        //write mjpegd eof, only used by snap mode
        Mjpegd_Frame_WriteEOF(frame,(u8_t*)Http_ChunkedEOF,MJPEGD_CHRARR_STRLEN(Http_ChunkedEOF));
    }
    catch(NULL_FRAME)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("ProcessRawFrame NULL_FRAME\n"));
    }
    catch(BAD_FRAME)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("ProcessRawFrame BAD_FRAME\n"));
    }
    catch(ALREADY_PROCESSED)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("ProcessRawFrame ALREADY_PROCESSED\n"));
    }
    finally
    {
        return;
    }

}
