#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/mjpegd_stream.h"
#include "trycatch_while.h"

#include "alloca.h"
#include "lwip/tcp.h"
#include "eth/apps/mjpeg/mjpegd_framebuf.h"


#define MJPEGD_STREAM_LIMIT MJPEGD_TOTAL_CLEINT_LIMIT

//TODO:Collect this
volatile static u8_t stream_client_count=0;

u8_t mjpegd_stream_get_client_count(void)
{
    return stream_client_count;
}

/**
 * @brief Force output to all stream clients
 * @return err_t ERR_OK
 */
void mjpegd_stream_output(void)
{
    client_state_t* cs;
    err_t err;

    for (cs=mjpegd_get_clients(); cs!=NULL; cs=cs->_next)
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
                        DBG_ARG("stream_output TCP_OUTPUT %s\n",lwip_strerr(err)));
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
err_t mjpegd_nextframe_stream(void* client_state)
{
    err_t err;
    client_state_t* cs = (client_state_t*)client_state;

    try
    {
        throwif(cs==NULL,NULL_CS);

        Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf,cs->frame);
        cs->frame = NULL;

        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("stream NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    finally
    {
        return err;
    }
}

err_t mjpegd_stream_recv_request(void* client_state)
{
    err_t err;
    client_state_t* cs = (client_state_t*)client_state;
    try
    {
        throwif(cs==NULL, NULL_CS);
        throwif(cs->request_handler==NULL, NULL_REQUEST);
        throwif(cs->request_handler->req != REQUEST_STREAM, REQUEST_NOT_MATCH);
        throwif(stream_client_count+1>MJPEGD_STREAM_LIMIT,TOO_MANY_CLIENTS);
        stream_client_count++;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("stream_recv_request new client, total: %d\n",stream_client_count));
        err = ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("stream_recv_request NULL_CS"));
        err = ERR_ARG;
    }
    catch(NULL_REQUEST)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("stream_recv_request NULL_REQUEST"));
        err = ERR_ARG;
    }
    catch(REQUEST_NOT_MATCH)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("stream_recv_request REQUEST_NOT_MATCH"));
        err = ERR_ARG;
    }
    catch(TOO_MANY_CLIENTS)
    {
        err = ERR_USE;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("stream_recv_request TOO_MANY_CLIENTS, total: %d\n",stream_client_count));
    }
    finally
    {
        return err;
    }
}

err_t mjpegd_stream_clsd_request(void* client_state)
{
    err_t err;
    client_state_t* cs = (client_state_t*)client_state;
    try
    {
        throwif(cs==NULL, NULL_CS);
        throwif(cs->request_handler==NULL, NULL_REQUEST);
        throwif(cs->request_handler->req != REQUEST_STREAM, REQUEST_NOT_MATCH);
        throwif(stream_client_count==0,CLIENT_COUNT_ZERO);
        
        stream_client_count--;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("stream_clsd_request clsd client, total: %d\n",stream_client_count));
        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("stream_clsd_request NULL_CS"));
        err = ERR_ARG;
    }
    catch(NULL_REQUEST)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("stream_clsd_request NULL_REQUEST"));
        err = ERR_ARG;
    }
    catch(REQUEST_NOT_MATCH)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("stream_clsd_request REQUEST_NOT_MATCH"));
        err = ERR_ARG;
    }
    catch(CLIENT_COUNT_ZERO)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("stream_clsd_request CLIENT_COUNT_ZERO"));
        err = ERR_ARG;
    }
    finally
    {
        return err;
    }
}

