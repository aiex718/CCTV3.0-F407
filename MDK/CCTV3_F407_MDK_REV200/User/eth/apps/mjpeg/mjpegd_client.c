#include "eth/apps/mjpeg/mjpegd_client.h"
#include "eth/apps/mjpeg/mjpegd_opts.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/mjpegd_framebuf.h"
#include "eth/apps/mjpeg/trycatch.h"

#include "lwip/mem.h"
#include "lwip/tcp.h"

ClientState_t* Mjpegd_Client_New(Mjpegd_t *mjpegd, struct tcp_pcb *pcb)
{
    ClientState_t *cs = NULL;
    try
    {
        throwif(pcb == NULL,PCB_NULL);
        throwif(mjpegd == NULL,MJPEG_NULL);

        cs = (ClientState_t*)mem_malloc(sizeof(ClientState_t));
        throwif(cs == NULL,CLIENT_STATE_OOM);
        
        MJPEGD_MEMSET(cs,0,sizeof(ClientState_t));
        cs->pcb = pcb;
        cs->parent_mjpeg = mjpegd;

        //Attach to mjpegd->_clients_list head
        cs->_next = mjpegd->_clients_list;
        mjpegd->_clients_list=cs;

        mjpegd->_client_count++;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("Alloc client %p, total %d\n",cs,mjpegd->_client_count));
    }
    catch(PCB_NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Client_New PCB_NULL\n"));
    }
    catch(MJPEG_NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Client_New MJPEG_NULL\n"));
    }
    catch(CLIENT_STATE_OOM)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Client_New No mem for new client\n"));
    }
    finally
    {
        return cs;
    }
}

void Mjpegd_Client_Free(ClientState_t *cs)
{
    if(cs!=NULL)
    {
        struct tcp_pcb *pcb = cs->pcb;
        Mjpegd_t *mjpegd = (Mjpegd_t*)cs->parent_mjpeg;

        ClientState_t **prev_ptr=NULL;
        ClientState_t *prev;
        
        //Remove cs from mjpeg->_clients_list
        //Try get previous node whose next is pointing to cs
        for (prev=mjpegd->_clients_list; prev!=NULL && prev->_next!=cs; prev=prev->_next);
        //if previous node is NULL, cs is first node in _clients_list
        //concat cs->next to _clients_list head directly, otherwise concat to prev->_next
        prev_ptr = prev==NULL ? &mjpegd->_clients_list : &prev->_next;
        *prev_ptr = cs->_next;

        Mjpegd_FrameBuf_Release(mjpegd->FrameBuf,cs->frame);
        cs->frame = NULL;

        mem_free(cs);

        if(pcb!=NULL)
            tcp_arg(pcb, NULL);//this isn't necessary, just for safety

        mjpegd->_client_count--;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("Dealloc client %p, remaning %d\n",cs,mjpegd->_client_count));
    }
}

err_t Mjpegd_Client_ParseRequest(ClientState_t *cs,const char* req,const u16_t req_len)
{
    err_t err;

    try
    {
        char *line_end;
        u8_t i;
        throwif (cs==NULL,NULL_CLIENT);

        line_end=MJPEGD_STRNSTR(req, "\r\n",req_len);
        throwif (req_len < MJPEGD_MIN_REQ_LEN || line_end==NULL, REQUEST_NOT_COMPLETE);
        throwif (MJPEGD_STRNCMP(req, "GET ", 4) , NOT_HTTP_GET);

        //default request handler is REQUEST_NOTFOUND
        cs->request_handler = &mjpegd_request_handlers[REQUEST_NOTFOUND];

        for ( i = 0; i < MJPEGD_ARRLEN(mjpegd_request_handlers); i++)
        {
            const request_handler_t* handler = &mjpegd_request_handlers[i];
            if (!MJPEGD_STRNCMP(req+4, handler->url, strlen(handler->url)))
            {
                cs->request_handler = handler;
                break;
            }
        }
        err=ERR_OK;
    }   
    catch(NULL_CLIENT)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("ParseRequest NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(REQUEST_NOT_COMPLETE)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("ParseRequest REQUEST_NOT_COMPLETE:%s\n",req));

        err=ERR_ARG;
    }
    catch(NOT_HTTP_GET)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("ParseRequest NOT_HTTP_GET:%s\n",req));

        err=ERR_ARG;
    }
    finally
    {
        return err;
    }

}

err_t Mjpegd_Client_BuildResponse(ClientState_t *cs)
{
    err_t err;

    try
    {
        throwif (cs==NULL,NULL_CS);
        throwif (cs->request_handler==NULL,NULL_REQUEST_HANDLER);

        client_assign_file(cs,
            (u8_t*)cs->request_handler->response,
            cs->request_handler->response_len);

        cs->get_nextfile_func = cs->request_handler->get_nextfile_func;
        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("response NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(NULL_REQUEST_HANDLER)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("response NULL_REQUEST_HANDLER\n"));

        err=ERR_ARG;
    }
    finally
    {
        return err;
    }
}
