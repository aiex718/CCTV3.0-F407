#include "eth/apps/mjpeg/mjpegd_stream.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/trycatch.h"
#include "eth/apps/mjpeg/mjpegd_framebuf.h"
#include "eth/apps/mjpeg/mjpegd_client.h"

#include "lwip/tcp.h"


#define MJPEGD_STREAM_LIMIT MJPEGD_TOTAL_CLEINT_LIMIT

/**
 * @brief Force output to all stream clients
 * @return err_t ERR_OK
 */
void Mjpegd_Stream_Output(Mjpegd_t *mjpegd)
{
    ClientState_t* cs;
    err_t err;

    for (cs=mjpegd->_clients_list; cs!=NULL; cs=cs->_next)
    {
        if( cs->conn_state==CS_RECEIVED && 
            cs->request_handler->req == REQUEST_STREAM &&
            cs->pcb!=NULL && 
            cs->pcb->unsent !=NULL)
        {
            err = tcp_output(cs->pcb);
            if(err != ERR_OK)
            {
                if(err!=ERR_BUF)
                {
                    LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                        MJPEGD_DBG_ARG("stream_output TCP_OUTPUT %s\n",lwip_strerr(err)));
                }
                return;
            }
        }
    }
}

/**
 * @brief Release frame when transfer finished, next frame will be 
 *        assigned when new frame arrived.
 * @return err_t ERR_OK
 */
err_t Mjpegd_Stream_NextFrame(void* client_state)
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

        Mjpegd_FrameBuf_Release(frame_buf,cs->frame);
        cs->frame = NULL;

        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_NextFrame NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(NULL_MJPEGD)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_NextFrame NULL_MJPEGD\n"));
        err=ERR_ARG;
    }
    catch(NULL_FRAMEBUF)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_NextFrame NULL_FRAMEBUF\n"));
        err=ERR_ARG;
    }
    finally
    {
        return err;
    }
}

err_t Mjpegd_Stream_RecvRequest(void* client_state)
{
    err_t err;
    ClientState_t *cs;
    Mjpegd_t *mjpegd;
    try
    {
        cs = (ClientState_t*)client_state;
        throwif(cs==NULL,NULL_CS);
        mjpegd = (Mjpegd_t*)cs->parent_mjpeg;
        throwif(mjpegd==NULL,NULL_MJPEGD);

        throwif(cs->request_handler==NULL, NULL_REQUEST);
        throwif(cs->request_handler->req != REQUEST_STREAM, REQUEST_NOT_MATCH);
        throwif(mjpegd->_stream_count+1>MJPEGD_STREAM_LIMIT,TOO_MANY_CLIENTS);

        mjpegd->_stream_count++;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("Stream_RecvRequest new client, total: %d\n",mjpegd->_stream_count));
        err = ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_RecvRequest NULL_CS"));
        err = ERR_ARG;
    }
    catch(NULL_MJPEGD)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_RecvRequest NULL_MJPEGD\n"));
        err=ERR_ARG;
    }
    catch(NULL_REQUEST)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_RecvRequest NULL_REQUEST"));
        err = ERR_ARG;
    }
    catch(REQUEST_NOT_MATCH)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_RecvRequest REQUEST_NOT_MATCH"));
        err = ERR_ARG;
    }
    catch(TOO_MANY_CLIENTS)
    {
        err = ERR_USE;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("Stream_RecvRequest TOO_MANY_CLIENTS, total: %d, limit: %d\n"
                ,mjpegd->_stream_count,MJPEGD_STREAM_LIMIT));
    }
    finally
    {
        return err;
    }
}

err_t Mjpegd_Stream_CloseRequest(void* client_state)
{
    err_t err;
    ClientState_t *cs;
    Mjpegd_t *mjpegd;
    try
    {
        cs = (ClientState_t*)client_state;
        throwif(cs==NULL,NULL_CS);
        mjpegd = (Mjpegd_t*)cs->parent_mjpeg;
        throwif(mjpegd==NULL,NULL_MJPEGD);

        throwif(cs->request_handler==NULL, NULL_REQUEST);
        throwif(cs->request_handler->req != REQUEST_STREAM, REQUEST_NOT_MATCH);
        throwif(mjpegd->_stream_count==0,CLIENT_COUNT_ZERO);
        
        mjpegd->_stream_count--;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("Stream_CloseRequest clsd client, total: %d\n",mjpegd->_stream_count));
        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_CloseRequest NULL_CS"));
        err = ERR_ARG;
    }
    catch(NULL_MJPEGD)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_CloseRequest NULL_MJPEGD\n"));
        err=ERR_ARG;
    }
    catch(NULL_REQUEST)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_CloseRequest NULL_REQUEST"));
        err = ERR_ARG;
    }
    catch(REQUEST_NOT_MATCH)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_CloseRequest REQUEST_NOT_MATCH"));
        err = ERR_ARG;
    }
    catch(CLIENT_COUNT_ZERO)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_CloseRequest CLIENT_COUNT_ZERO"));
        err = ERR_ARG;
    }
    finally
    {
        return err;
    }
}

