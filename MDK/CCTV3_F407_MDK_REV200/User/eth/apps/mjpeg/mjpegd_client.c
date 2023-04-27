#include "eth/apps/mjpeg/mjpegd_client.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/mjpegd_framepool.h"
#include "eth/apps/mjpeg/trycatch.h"

#include "lwip/mem.h"
#include "lwip/tcp.h"

ClientState_t *Mjpegd_Client_New(Mjpegd_t *mjpegd, struct tcp_pcb *pcb)
{
    ClientState_t *cs = NULL;
    try
    {
        throwif(pcb == NULL, PCB_NULL);
        throwif(mjpegd == NULL, MJPEG_NULL);
        throwif(mjpegd->_client_count+1>MJPEGD_TOTAL_CLIENT_LIMIT, CLIENT_LIMIT);

        cs = (ClientState_t *)mem_malloc(sizeof(ClientState_t));
        throwif(cs == NULL, CLIENT_STATE_OOM);

        MJPEGD_MEMSET(cs, 0, sizeof(ClientState_t));
        cs->pcb = pcb;
        cs->parent_mjpeg = mjpegd;

        // Attach to mjpegd->_clients_list head
        cs->_next = mjpegd->_clients_list;
        mjpegd->_clients_list = cs;

        mjpegd->_client_count++;
        mjpegd->_idle_timer = sys_now();
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
                    MJPEGD_DBG_ARG("Alloc client %p, total %d\n", cs, mjpegd->_client_count));
    }
    catch (PCB_NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    MJPEGD_DBG_ARG("Client_New PCB_NULL\n"));
    }
    catch (MJPEG_NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    MJPEGD_DBG_ARG("Client_New MJPEG_NULL\n"));
    }
    catch (CLIENT_LIMIT)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                    MJPEGD_DBG_ARG("CLIENT_LIMIT reached %d\n",MJPEGD_TOTAL_CLIENT_LIMIT));
    }
    catch (CLIENT_STATE_OOM)
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
    if (cs != NULL)
    {
        struct tcp_pcb *pcb = cs->pcb;
        Mjpegd_t *mjpegd = (Mjpegd_t *)cs->parent_mjpeg;

        ClientState_t **prev_ptr = NULL;
        ClientState_t *prev;

        // Remove cs from mjpeg->_clients_list
        // Try get previous node whose next is pointing to cs
        for (prev = mjpegd->_clients_list; prev != NULL && prev->_next != cs; prev = prev->_next);
        // if previous node is NULL, cs is first node in _clients_list
        // concat cs->next to _clients_list head directly, otherwise concat to prev->_next
        prev_ptr = prev == NULL ? &mjpegd->_clients_list : &prev->_next;
        *prev_ptr = cs->_next;

        Mjpegd_FramePool_Release(mjpegd->FramePool, cs->frame);
        cs->frame = NULL;

        mem_free(cs);

        if (pcb != NULL)
            tcp_arg(pcb, NULL); // this isn't necessary, just for safety

        mjpegd->_client_count--;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE | LWIP_DBG_LEVEL_WARNING,
                    MJPEGD_DBG_ARG("Dealloc client %p, remaning %d\n", cs, mjpegd->_client_count));
    }
}
