#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "lwip/mem.h"

#include "trycatch_while.h"

#include "eth/apps/mjpeg/mjpegd_snap.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"

#include "eth/apps/mjpeg/mjpegd_framebuf.h"

err_t mjpegd_nextframe_snap_start(void* client_state)
{
    err_t err;
    client_state_t* cs = (client_state_t*)client_state;
    try
    {
        u8_t w_len=0;
        throwif(cs==NULL,NULL_CS);

        //release old frame if any, should not happen
        Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf,cs->frame);
        cs->frame = NULL;

        //try get a new frame
        cs->frame = Mjpegd_FrameBuf_GetLatest(Mjpegd_FrameBuf,0);
        throwif(cs->frame==NULL,GET_FRAME_FAIL);
        throwif(!Mjpegd_Frame_IsValid(cs->frame),BAD_FRAME);

        if(cs->buffer == NULL)
            cs->buffer = (u8_t*)mem_malloc(10);

        throwif(cs->buffer==NULL,BUFFER_ALLOC_FAIL);
        
        //write chunked frame head
        w_len = sprintf((char*)cs->buffer,"%x\r\n",cs->frame->payload_len);

        //assign file for sending
        client_assign_file(cs,cs->buffer,w_len);

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
            DBG_ARG("snap_start sending %d bytes\n", cs->file_len));

        //regist next file for sending chunked body(jpeg file content)
        cs->get_nextfile_func=mjpegd_nextframe_snap_body;
        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("snap_start NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(GET_FRAME_FAIL)
    {
        //frame not available, maybe camera not started yet
        //try again later
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("snap_start GET_FRAME_FAIL, try later\n"));

        err=ERR_INPROGRESS;
    }
    catch(BAD_FRAME)
    {
        //bad frame,release and try again later
        Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf,cs->frame);
        cs->frame = NULL;

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("snap_start BAD_FRAME, try later\n"));

        err=ERR_INPROGRESS;
    }
    catch(BUFFER_ALLOC_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("snap_start BUFFER_ALLOC_FAIL\n"));
        err=ERR_BUF;
    }
    finally
    {
        return err;
    }
}

err_t mjpegd_nextframe_snap_body(void *client_state)
{
    err_t err;
    client_state_t* cs = (client_state_t*)client_state;
    try
    {
        throwif(cs==NULL,NULL_CS);
        throwif(cs->frame==NULL,NULL_FRAME);
        throwif(cs->buffer==NULL,NULL_BUFFER);

        //chunked head send finish
        //free buffer here
        mem_free(cs->buffer);
        cs->buffer=NULL;

        //assign file for sending
        client_assign_file(cs,
            cs->frame->payload,
            cs->frame->payload_len);

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
            DBG_ARG("snap_body sending %d bytes\n", cs->file_len));

        //regist next file for sending chunked eof
        cs->get_nextfile_func=mjpegd_nextframe_snap_finish;
        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("snap_body NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(NULL_FRAME)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("snap_body NULL_FRAME\n"));

        err=ERR_ARG;
    }
    catch(NULL_BUFFER)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("snap_body NULL_BUFFER\n"));

        err=ERR_BUF;
    }
    finally
    {
        return err;
    }
}

err_t mjpegd_nextframe_snap_finish(void *client_state)
{
    static const char Http_ChunkedEOF[]="\r\n0\r\n\r\n";
    err_t err;
    client_state_t* cs = (client_state_t*)client_state;
    try
    {
        throwif(cs==NULL,NULL_CS);
        throwif(cs->frame==NULL,NULL_FRAME);
        
        //release previous frame if exist
        Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf,cs->frame);
        cs->frame = NULL;

        //write chunked end
        client_assign_file(cs,(u8_t*)Http_ChunkedEOF,MJPEGD_STRLEN(Http_ChunkedEOF));

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
            DBG_ARG("snap_finish sending %d bytes\n", cs->file_len));

        //clear nextfile, we're all done sending snap 
        cs->get_nextfile_func=NULL;
        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("snap_finish NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(NULL_FRAME)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("snap_finish NULL_FRAME\n"));
        err=ERR_ARG;
    }
    finally
    {
        return err;
    }
}
