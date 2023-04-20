#include "eth/apps/mjpeg/mjpegd_debug.h"

#include "eth/apps/mjpeg/mjpegd_client.h"
#include "eth/apps/mjpeg/trycatch_while.h"
#include "string.h"

#include "lwip/mem.h"
#include "lwip/tcp.h"
#include "eth/apps/mjpeg/mjpegd_framebuf.h"

//TODO:Collect this
static volatile u8_t mjpegd_client_count=0;
static client_state_t *clients_list=NULL;

client_state_t* mjpegd_get_clients(void)
{
    return clients_list;
}

u8_t mjpegd_get_client_count(void)
{
    return mjpegd_client_count;
}

client_state_t* mjpegd_new_client(struct tcp_pcb *pcb)
{
    client_state_t *cs = NULL;
    try
    {
        cs = (client_state_t*)mem_malloc(sizeof(client_state_t));
        throwif(cs == NULL,CLIENT_STATE_OOM);
        memset(cs,0,sizeof(client_state_t));
        cs->pcb = pcb;

        //Attach to clients_list head
        cs->_next = clients_list;
        clients_list=cs;

        mjpegd_client_count++;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("Alloc client %p, total %d\n",cs,mjpegd_client_count));
    }
    catch(CLIENT_STATE_OOM)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("No mem for new client\n"));
    }
    finally
    {
        return cs;
    }
}

err_t mjpegd_free_client(struct tcp_pcb *pcb,client_state_t *cs)
{
    if(cs!=NULL)
    {
        u32_t client_addr=(u32_t)cs;
        client_state_t **prev_ptr=NULL;
        client_state_t *prev;
        
        //Get prev and delete cs
        for (prev=clients_list; prev!=NULL && prev->_next!=cs; prev=prev->_next);
        
        //if prev is NULL, cs is first node in clients_list
        //concat cs->next to head directly, otherwise concat to prev->_next
        prev_ptr = prev==NULL ? &clients_list : &prev->_next;
        *prev_ptr = cs->_next;

        Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf,cs->frame);
        cs->frame = NULL;
        
        if(cs->buffer != NULL)
        {
            mem_free(cs->buffer);
            cs->buffer=NULL;
        }

        mem_free(cs);

        if(pcb!=NULL)
            tcp_arg(pcb, NULL);//this might not be necessary, just for robust

        mjpegd_client_count--;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("Dealloc client %x, remaning %d\n",client_addr,mjpegd_client_count));
    }

    return ERR_OK;
}

