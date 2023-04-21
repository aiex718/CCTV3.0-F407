#include "eth/apps/mjpeg/mjpegd_frameproc.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/mjpegd_opts.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/trycatch.h"



//private functions
static void Mjpegd_FrameProc_ProcessRawFrame(Mjpegd_Frame_t* frame);

/**
 * @brief Return new frame captured by camera, return next frame buffer
 *        for next capture if available.
 * @details If previous frame is not yet processed, we drop previous frame 
 *          and reuse the frame buffer for next capture.          
 *          Otherwise, get a new frame buffer for next capture.
 *          If no frame buffer available, return null.
 * @param frame New jpeg frame captured by camera.
 * @return Next frame buffer for next capture, null if not available.
 */
Mjpegd_Frame_t* Mjpegd_FrameProc_NextFrame(Mjpegd_t *mjpeg,Mjpegd_Frame_t* frame)
{
    Mjpegd_Frame_t *next_frame=NULL;

    if (Mjpegd_Frame_IsValid(frame)==false)
    {
        //Frame null usually occurs when first capture from camera 
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("NextFrame bad frame %p\n",frame));
        next_frame = frame; //reuse the frame buffer
    }
    else
    {
        next_frame = (Mjpegd_Frame_t*)
            MJPEGD_ATOMIC_XCHG((__IO u32_t *)&mjpeg->_pending_frame,(u32_t)frame);
    }

    if(next_frame!=NULL)
    {
        mjpeg->_drop_counter++;
        if(mjpeg->_drop_counter >= MJPEGD_FRAMEDROP_WARNING_THRESHOLD)
        {
            LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_LEVEL_WARNING, 
                MJPEGD_DBG_ARG("NextFrame dropped %d, proc too slow?\n",mjpeg->_drop_counter));
            mjpeg->_drop_counter=0;
        }
        Mjpegd_Frame_Clear(next_frame);
    }
    else
    {
        next_frame=Mjpegd_FrameBuf_GetIdle(mjpeg->FrameBuf);
    }

    return next_frame;
}

void Mjpegd_FrameProc_ProcPending(Mjpegd_t *mjpeg)
{
    MJPEGD_SYSTIME_T now = sys_now();
    //take pending frame to local and process it
    Mjpegd_Frame_t* local_frame = (Mjpegd_Frame_t*)
            MJPEGD_ATOMIC_XCHG((__IO u32_t *)&mjpeg->_pending_frame,NULL);
    
    if(local_frame!=NULL)
    {
        Mjpegd_FrameProc_ProcessRawFrame(local_frame);
        Mjpegd_FrameBuf_ReleaseIdle(mjpeg->FrameBuf,local_frame);

        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_TRACE, 
            MJPEGD_DBG_ARG("ProcPending %p, _sem=%d\n",local_frame,local_frame->_sem));
        
        mjpeg->_fps_counter++;
    }

    //show fps
    if(now - mjpeg->_fps_timer > (1000<<MJPEGD_FPS_PERIOD))
    {
        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_STATE, 
            MJPEGD_DBG_ARG("Proc fps %d\n",(mjpeg->_fps_counter>>MJPEGD_FPS_PERIOD)));
        mjpeg->_fps_counter = 0;
        mjpeg->_fps_timer = now;
    }
}


/**
 * @brief Add headers, comments(contains frame time information) and HTTP EOF to received frame.
 * @note  This function should regist to FrameBuffer_ReceiveRawFrame_Event.
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
        w_len = sprintf((char*)buf,Http_Mjpeg_ContentLength,frame->payload_len);
        Mjpegd_Frame_WriteHeader(frame,buf,w_len);
        Mjpegd_Frame_WriteHeader(frame,(u8_t*)Http_Mjpeg_ContentType,MJPEGD_CHRARR_STRLEN(Http_Mjpeg_ContentType));
        Mjpegd_Frame_WriteHeader(frame,(u8_t*)Http_Mjpeg_Boundary,MJPEGD_CHRARR_STRLEN(Http_Mjpeg_Boundary));
        w_len = sprintf((char*)buf,"%x\r\n", Mjpegd_Frame_HeaderSize(frame) + frame->payload_len);
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
