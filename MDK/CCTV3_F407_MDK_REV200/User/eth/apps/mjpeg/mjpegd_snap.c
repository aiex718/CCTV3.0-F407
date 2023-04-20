#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "lwip/mem.h"

#include "trycatch_while.h"

#include "eth/apps/mjpeg/mjpegd_snap.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"

#include "eth/apps/mjpeg/mjpegd_framebuf.h"

err_t mjpegd_nextframe_snap(void* client_state)
{
    err_t err;
    client_state_t* cs = (client_state_t*)client_state;
    try
    {
        throwif(cs==NULL,NULL_CS);

        //release old frame if any, should not happen
        Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf,cs->frame);
        cs->frame = NULL;

        //try get a new frame
        cs->frame = Mjpegd_FrameBuf_GetLatest(Mjpegd_FrameBuf,0);
        throwif(cs->frame==NULL,GET_FRAME_FAIL);
        throwif(!Mjpegd_Frame_IsValid(cs->frame),BAD_FRAME);

        client_assign_file(cs,
            cs->frame->head,
            Mjpegd_Frame_SnapSize(cs->frame));

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
            DBG_ARG("nextframe_snap sending %d bytes\n", cs->file_len));

        cs->get_nextfile_func=NULL;
        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("nextframe_snap NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(GET_FRAME_FAIL)
    {
        //frame not available, maybe camera not started yet
        //try again later
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("nextframe_snap GET_FRAME_FAIL, try later\n"));

        err=ERR_INPROGRESS;
    }
    catch(BAD_FRAME)
    {
        //bad frame,release and try again later
        Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf,cs->frame);
        cs->frame = NULL;

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("nextframe_snap BAD_FRAME, try later\n"));

        err=ERR_INPROGRESS;
    }
    finally
    {
        return err;
    }
}
