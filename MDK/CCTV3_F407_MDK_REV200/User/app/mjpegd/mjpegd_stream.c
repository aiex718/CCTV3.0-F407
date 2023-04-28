#include "app/mjpegd/mjpegd_stream.h"
#include "app/mjpegd/mjpegd_opts.h"
#include "app/mjpegd/mjpegd_debug.h"
#include "app/mjpegd/trycatch.h"
#include "app/mjpegd/mjpegd_framepool.h"
#include "app/mjpegd/mjpegd_client.h"
#include "app/mjpegd/mjpegd_request.h"

#include "lwip/tcp.h"

/**
 * @brief Force output to all stream clients
 * @return err_t ERR_OK
 */
void Mjpegd_Stream_Output(Mjpegd_t *mjpegd)
{
    MJPEGD_SYSTIME_T now = sys_now();
    ClientState_t* cs;
    err_t err;

    for (cs=mjpegd->_clients_list; cs!=NULL; cs=cs->_next)
    {
        if( cs->conn_state==CS_RECEIVED && 
            cs->pcb!=NULL && 
            cs->request_handler !=NULL &&
            cs->pcb->unsent !=NULL &&
            (cs->request_handler->req == REQUEST_STREAM || 
            cs->request_handler->req == REQUEST_SNAP))
        {
            //force output
            err = tcp_output(cs->pcb);

            if(err==ERR_BUF)
            {
                ;//do nothing, try in next loop
            }
            else if(err != ERR_OK)
            {
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                    MJPEGD_DBG_ARG("output closing cs %p TCP_ERR %s\n",
                        cs,lwip_strerr(err)));
                cs->conn_state = CS_TCP_ERR;
            }
        }
    }
}

/**
 * @brief Release frame when transfer finished, next frame will be 
 *        assigned when new frame arrived.
 * @return err_t ERR_OK
 */
err_t Mjpegd_Stream_FrameSent(ClientState_t *cs)
{
    err_t err;
    Mjpegd_t *mjpegd;
    Mjpegd_FramePool_t *frame_pool;

    try
    {
        throwif(cs==NULL,NULL_CS);
        mjpegd = (Mjpegd_t*)cs->parent_mjpeg;
        throwif(mjpegd==NULL,NULL_MJPEGD);
        frame_pool = mjpegd->FramePool;
        throwif(frame_pool==NULL,NULL_FRAMEPOOL);
        throwif(cs->request_handler==NULL,NULL_REQUEST_HANDLER);
        
        //clear client file
        client_assign_file(cs,NULL,0);
        err=ERR_INPROGRESS;

        if(cs->frame!=NULL)
        {
            Mjpegd_FramePool_Release(frame_pool,cs->frame);
            cs->frame = NULL;

            //if client is a snap reques and frame not null
            //we already done transfer 1 frame
            //clear get_nextfile_func and return ERR_CLSD
            if(cs->request_handler->req == REQUEST_SNAP)
            {
                cs->get_nextfile_func=NULL;
                err = ERR_CLSD;
            }
        }   
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
    catch(NULL_FRAMEPOOL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_NextFrame NULL_FRAMEPOOL\n"));
        err=ERR_ARG;
    }
    catch(NULL_REQUEST_HANDLER)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Stream_NextFrame NULL_REQUEST_HANDLER\n"));
        err=ERR_ARG;
    }
    finally
    {
        return err;
    }
}

err_t Mjpegd_Stream_RecvRequest(ClientState_t *cs)
{
    err_t err;
    Mjpegd_t *mjpegd;
    try
    {
        throwif(cs==NULL,NULL_CS);
        mjpegd = (Mjpegd_t*)cs->parent_mjpeg;
        throwif(mjpegd==NULL,NULL_MJPEGD);

        throwif(cs->request_handler==NULL, NULL_REQUEST);
        throwif(cs->request_handler->req != REQUEST_STREAM, REQUEST_NOT_MATCH);
        throwif(mjpegd->_stream_count+1>MJPEGD_STREAM_CLIENT_LIMIT,TOO_MANY_CLIENTS);

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
                ,mjpegd->_stream_count,MJPEGD_STREAM_CLIENT_LIMIT));
    }
    finally
    {
        return err;
    }
}

err_t Mjpegd_Stream_CloseRequest(ClientState_t *cs)
{
    err_t err;
    Mjpegd_t *mjpegd;
    try
    {
        throwif(cs==NULL,NULL_CS);
        mjpegd = (Mjpegd_t*)cs->parent_mjpeg;
        throwif(mjpegd==NULL,NULL_MJPEGD);

        throwif(cs->request_handler==NULL, NULL_REQUEST);
        throwif(cs->request_handler->req != REQUEST_STREAM, REQUEST_NOT_MATCH);
        throwif(mjpegd->_stream_count==0,CLIENT_COUNT_ZERO);
        
        mjpegd->_stream_count--;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("close stream client, total: %d\n",mjpegd->_stream_count));
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

