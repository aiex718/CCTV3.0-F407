#include "eth/apps/mjpeg/mjpegd_snap.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/trycatch.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/mjpegd_framebuf.h"
#include "eth/apps/mjpeg/mjpegd_client.h"
#include "lwip/mem.h"

err_t Mjpegd_Snap_NextFrame(void* client_state)
{
    err_t err;
    ClientState_t *cs;
    Mjpegd_t *mjpegd;
    Mjpegd_FrameBuf_t *frame_buf;
    try
    {
        cs = (ClientState_t*)client_state;
        throwif(cs==NULL,NULL_CS);
        mjpegd = (Mjpegd_t*)cs->parent_mjpeg;
        throwif(mjpegd==NULL,NULL_MJPEGD);
        frame_buf = mjpegd->FrameBuf;
        throwif(frame_buf==NULL,NULL_FRAMEBUF);

        //release old frame if any, should not happen
        Mjpegd_FrameBuf_Release(frame_buf,cs->frame);
        cs->frame = NULL;

        //try get a new frame
        cs->frame = Mjpegd_FrameBuf_GetLatest(frame_buf,0);
        throwif(cs->frame==NULL,GET_FRAME_FAIL);
        throwif(!Mjpegd_Frame_IsValid(cs->frame),BAD_FRAME);

        client_assign_file(cs,
            cs->frame->head,
            Mjpegd_Frame_SnapSize(cs->frame));

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
            MJPEGD_DBG_ARG("Snap_NextFrame sending %d bytes\n", cs->file_len));

        cs->get_nextfile_func=NULL;
        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Snap_NextFrame NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(NULL_MJPEGD)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Snap_NextFrame NULL_MJPEGD\n"));
        err=ERR_ARG;
    }
    catch(NULL_FRAMEBUF)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Snap_NextFrame NULL_FRAMEBUF\n"));
        err=ERR_ARG;
    }
    catch(GET_FRAME_FAIL)
    {
        //frame not available, maybe camera not started yet
        //try again later
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("Snap_NextFrame GET_FRAME_FAIL, try later\n"));

        err=ERR_INPROGRESS;
    }
    catch(BAD_FRAME)
    {
        //bad frame,release and try again later
        Mjpegd_FrameBuf_Release(frame_buf,cs->frame);
        cs->frame = NULL;

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Snap_NextFrame BAD_FRAME, try later\n"));

        err=ERR_INPROGRESS;
    }
    finally
    {
        return err;
    }
}
