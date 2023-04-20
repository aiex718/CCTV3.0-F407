#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/mjpegd_stream.h"
#include "trycatch_while.h"

#include "alloca.h"
#include "lwip/tcp.h"
#include "bsp/sys/systime.h"
#include "eth/apps/mjpeg/mjpegd_framebuf.h"


#define MJPEGD_STREAM_LIMIT MJPEGD_TOTAL_CLEINT_LIMIT

//TODO:Collect this
volatile static u8_t stream_client_count=0;

u8_t mjpegd_stream_get_client_count(void)
{
    return stream_client_count;
}

//TODO:dont use alloca?
void mjpegd_stream_output(void)
{
    typedef struct client_time_pair_struct
    {
        client_state_t *client;
        u32_t transfer_time_diff;
    }client_time_pair_t;

    //order by transfer_time_diff, descending
    SysTime_t now = SysTime_Get();
    u8_t client_count = mjpegd_get_client_count();
    client_time_pair_t *client_time_pairs = (client_time_pair_t*)alloca(sizeof(client_time_pair_t)*client_count);
    client_time_pair_t temp;
    client_state_t* cs;
    u8_t i,j;
    err_t err;

    for (i=0,cs=mjpegd_get_clients(); cs!=NULL; i++,cs=cs->_next)
    {
        client_time_pairs[i].client = cs;
        client_time_pairs[i].transfer_time_diff = now - cs->previous_transfer_time;
    }

    //order by desc
    for (i = 0; i < client_count; ++i) 
    {
        for (j = i + 1; j < client_count; ++j)
        {
            if (client_time_pairs[i].transfer_time_diff < client_time_pairs[j].transfer_time_diff) 
            {
                temp =  client_time_pairs[i];
                client_time_pairs[i] = client_time_pairs[j];
                client_time_pairs[j] = temp;
            }
        }
    }

    for (i = 0; i < client_count; i++)
    {
        cs = client_time_pairs[i].client;
        if( cs->conn_state==CS_RECEIVED && 
            cs->request_handler->req == REQUEST_STREAM &&
            cs->pcb!=NULL && 
            cs->pcb->unsent !=NULL)
        {
            err = tcp_output(cs->pcb);
            if(err == ERR_OK)
                cs->previous_transfer_time = SysTime_Get();
            else
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

