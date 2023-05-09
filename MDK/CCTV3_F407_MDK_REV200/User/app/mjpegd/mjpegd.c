#include "app/mjpegd/mjpegd.h"

#include "lwip/tcp.h"
#include "lwip/timeouts.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"

#include "app/mjpegd/mjpegd_debug.h"
#include "app/mjpegd/mjpegd_memutils.h"
#include "app/mjpegd/trycatch.h"

//sub modules
#include "app/mjpegd/mjpegd_frameproc.h"
#include "app/mjpegd/mjpegd_request.h"
#include "app/mjpegd/mjpegd_stream.h"
#include "app/mjpegd/mjpegd_framepool.h"
#include "app/mjpegd/mjpegd_frame.h"
#include "app/mjpegd/mjpegd_client.h"
#include "app/mjpegd/mjpegd_camera.h"

/** Lwip raw api callbacks **/
static err_t Mjpegd_LwipAccept_Handler(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t Mjpegd_LwipRecv_Handler(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t Mjpegd_LwipSent_Handler(void *arg, struct tcp_pcb *pcb, u16_t len);
static err_t Mjpegd_LwipPoll_Handler(void *arg, struct tcp_pcb *pcb);
static void  Mjpegd_LwipErr_Handler(void *arg, err_t err);


/** mjpegd functions **/
static err_t Mjpegd_SendData(struct tcp_pcb *pcb, ClientState_t *cs);
static void Mjpegd_CloseConn(struct tcp_pcb *pcb, ClientState_t *cs);
static void Mjpegd_CheckIdle(Mjpegd_t *mjpegd);
static void Mjpegd_ShowFps(Mjpegd_t *mjpegd);
static void Mjpegd_ShowDrop(Mjpegd_t *mjpegd);

/** callbacks **/
static void Mjpegd_RecvNewFrame_handler(void *sender, void *arg, void *owner);

err_t Mjpegd_Init(Mjpegd_t *mjpegd)
{
    struct tcp_pcb *pcb;
    err_t err;

    Mjpegd_FramePool_Init(mjpegd->FramePool);
    Mjpegd_Camera_Init(mjpegd->Camera,mjpegd);

    mjpegd->_main_pcb=NULL;
    mjpegd->_clients_list=NULL;
    mjpegd->_client_count=0;
    mjpegd->_stream_count=0;
    mjpegd->_pending_frame=NULL;
    mjpegd->_fps_timer=sys_now();
    mjpegd->_fps_counter=0;
    mjpegd->_drop_counter=0;
    mjpegd->_idle_timer=sys_now();

    try
    {
        pcb = tcp_new();
        throwif(pcb == NULL,NEW_PCB_FAIL);

        tcp_setprio(pcb, MJPEGD_MAIN_TCP_PRIO);

        err = tcp_bind(pcb, IP_ADDR_ANY, mjpegd->Port);
        throwif(err!=ERR_OK,BIND_FAIL);

        mjpegd->_main_pcb = tcp_listen(pcb);
        throwif(mjpegd->_main_pcb==NULL,LISTEN_FAIL);

        tcp_arg(mjpegd->_main_pcb, mjpegd);
        tcp_accept(mjpegd->_main_pcb, Mjpegd_LwipAccept_Handler);

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SEVERE 
            ,MJPEGD_DBG_ARG("init ok at port %d\n",mjpegd->Port));

        //regist frame_pool newframe callback
        mjpegd->RecvNewFrame_cb.func = Mjpegd_RecvNewFrame_handler;
        mjpegd->RecvNewFrame_cb.owner = mjpegd;
        Mjpegd_FramePool_SetCallback(mjpegd->FramePool,
            FRAMEPOOL_CALLBACK_RX_NEWFRAME,&mjpegd->RecvNewFrame_cb);

        //invoke service to start timeout loop and new snap
        Mjpegd_Service((void*)mjpegd);
        err=ERR_OK;
    }
    catch(NEW_PCB_FAIL)
    {
        err = ERR_MEM;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , MJPEGD_DBG_ARG("new pcb fail\n"));
    }
    catch(BIND_FAIL)
    {
        err = ERR_USE;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , MJPEGD_DBG_ARG("pcb bind fail\n"));
    }
    catch(LISTEN_FAIL)
    {
        err = ERR_MEM;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , MJPEGD_DBG_ARG("pcb listen fail\n"));
    }
    finally
    {
        if(mjpegd->_main_pcb==NULL && pcb!=NULL)
        {
            memp_free(MEMP_TCP_PCB, pcb);
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , MJPEGD_DBG_ARG("init failed, free orig pcb\n"));
        }
    }

    return err;
}

void Mjpegd_ConfigSet(Mjpegd_t *mjpegd,const Mjpegd_ConfigFile_t *config)
{
    mjpegd->Port = config->Mjpegd_Port;
}

void Mjpegd_ConfigExport(const Mjpegd_t *mjpegd,Mjpegd_ConfigFile_t *config)
{
    config->Mjpegd_Port = mjpegd->Port;
}

bool Mjpegd_IsConfigValid(Mjpegd_t *mjpegd,const Mjpegd_ConfigFile_t *config)
{
    if(config==NULL || config->Mjpegd_Port == 0 || config->Mjpegd_Port > 65535)
        return false;

    return true;
}

static err_t Mjpegd_LwipAccept_Handler(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    ClientState_t* cs;
    Mjpegd_t *mjpegd = (Mjpegd_t*)arg;
    char ip_str[IP4ADDR_STRLEN_MAX];

    try
    {
        throwif(err !=ERR_OK,ERR_ACCEPT_HANDLER_ERROR);
        throwif(mjpegd == NULL,FATAL_ERROR_NULL_MJPEG);
        throwif(newpcb == NULL,FATAL_ERROR_NULL_PCB);
        tcp_setprio(newpcb, MJPEGD_STREAM_TCP_PRIO);

        ip4addr_ntoa_r(&(newpcb->remote_ip), ip_str, sizeof(ip_str));

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
            MJPEGD_DBG_ARG("New client pcb %p %s:%d\n",
                newpcb,ip_str,newpcb->remote_port));

        cs = Mjpegd_Client_New(mjpegd,newpcb);
        throwif(cs==NULL,NEW_CLIENT_ERROR);
        
        cs->conn_state = CS_ACCEPTED;
        tcp_arg(newpcb, cs);
        tcp_recv(newpcb, Mjpegd_LwipRecv_Handler);
        tcp_err(newpcb, Mjpegd_LwipErr_Handler);
        tcp_poll(newpcb, Mjpegd_LwipPoll_Handler, MJPEGD_POLL_INTERVAL);
        tcp_sent(newpcb, Mjpegd_LwipSent_Handler);
        //tcp_nagle_disable(newpcb);//disable nagle for minimal latency

        err = ERR_OK;
    }
    catch(ERR_ACCEPT_HANDLER_ERROR)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("accept ERR_ACCEPT_HANDLER_ERROR\n"));
    }
    catch(FATAL_ERROR_NULL_MJPEG)
    {
        err = ERR_ARG;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("accept mjpeg NULL\n"));
    }
    catch(FATAL_ERROR_NULL_PCB)
    {
        err = ERR_ARG;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("accept Main or accept pcb NULL\n"));
    }
    catch(NEW_CLIENT_ERROR)
    {
        err = ERR_MEM;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("accept New client fail\n"));
    }
    finally
    {
        return err;
    }
}

static err_t Mjpegd_LwipRecv_Handler(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{   
    ClientState_t *cs = (ClientState_t*) arg;
    
    try
    {
        throwif(cs == NULL,BAD_CLIENT_NULL);
        //if recv pbuf == null, client close conn
        throwif(p == NULL,CONN_CLOSED);
        tcp_recved(pcb, p->tot_len);

        throwif(err != ERR_OK,ERR_CONN_ERROR);
        
        //make sure it's first time we receive data
        //we dont support socket reuse(HTTP persistent connection)
        throwif(cs->conn_state!=CS_ACCEPTED,BAD_CLIENT_MULTI_REQUEST);
        cs->conn_state = CS_RECEIVED;
        #if MJPEGD_DEBUG_PRINT_PAYLOAD
            *((char*)(p->payload) + p->len) = '\0';
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE , 
                MJPEGD_DBG_ARG("Received payload:\n%s",(const char*)p->payload));
        #endif

        if (p->len != p->tot_len) 
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
                MJPEGD_DBG_ARG("incomplete requset\n"));
        }

        err = Mjpegd_Request_Parse(cs,p->payload,p->len);
        throwif(err != ERR_OK,ERR_PARSE_REQUEST_FAIL);
        throwif(cs->request_handler==NULL,ERR_NULL_REQUEST_HANDLER);

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE, 
            MJPEGD_DBG_ARG("Received request:%s\n",mjpegd_strreq(cs->request_handler->req)));
        
        if(cs->request_handler->recv_request_func !=NULL)
            err = cs->request_handler->recv_request_func(cs);

        if(err==ERR_USE)//too many connection
            cs->request_handler = &mjpegd_request_handlers[REQUEST_TOOMANY];
        else
            throwif(err != ERR_OK,ERR_RECV_REQUEST_FAIL);
        
        err = Mjpegd_Request_BuildResponse(cs);
        throwif(err!=ERR_OK,ERR_BUILD_RESPONSE_FAIL);
        
        err = Mjpegd_SendData(pcb,cs);
        throwif(err!=ERR_OK,ERR_SEND_DATA);
    }
    catch(CONN_CLOSED)
    {   
        char ip_str[IP4ADDR_STRLEN_MAX];
        ip4addr_ntoa_r(&(pcb->remote_ip), ip_str, sizeof(ip_str));

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
            MJPEGD_DBG_ARG("recv Client closed %s:%d\n",ip_str,pcb->remote_port));
        err=ERR_CLSD;
    }
    catch(ERR_CONN_ERROR)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("recv ERR_CONN_ERROR %s\n", lwip_strerr(err)));
    }
    catch(BAD_CLIENT_NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("recv BAD_CLIENT_NULL\n"));
        err=ERR_ARG;
    }
    catch(BAD_CLIENT_MULTI_REQUEST)
    {
        char ip_str[IP4ADDR_STRLEN_MAX];
        ip4addr_ntoa_r(&(pcb->remote_ip), ip_str, sizeof(ip_str));

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("recv BAD_CLIENT_MULTI_REQUEST %s:%d\n", ip_str,pcb->remote_port));
        err=ERR_OK;
    }
    catch(ERR_PARSE_REQUEST_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("recv ERR_PARSE_REQUEST_FAIL %s\n", lwip_strerr(err)));
    }
    catch(ERR_NULL_REQUEST_HANDLER)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("recv ERR_NULL_REQUEST_HANDLER %s\n", lwip_strerr(err)));
    }
    catch(ERR_RECV_REQUEST_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("recv ERR_RECV_REQUEST_CALLBACK_FAIL %s\n", lwip_strerr(err)));
    }
    catch(ERR_BUILD_RESPONSE_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("recv ERR_INIT_RESPONSE_FAIL %s\n", lwip_strerr(err)));
    }
    catch(ERR_SEND_DATA)
    {
        //this is our first time send data, should not fail
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("recv ERR_SEND_DATA_FAIL %s\n", lwip_strerr(err)));
    }
    finally
    {
        if (p != NULL) //we dont store any pbuf 
            pbuf_free(p);

        if(err!=ERR_OK)
            Mjpegd_CloseConn(pcb,cs);

        return err;
    }
}

static err_t Mjpegd_LwipSent_Handler(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    //client acked, confirm received data
    //continue send data
    err_t err;
    ClientState_t *cs = (ClientState_t*) arg;
    LWIP_UNUSED_ARG(len);

    try
    {
        throwif(cs == NULL,BAD_CLIENT_NULL);
        throwif(cs->conn_state != CS_RECEIVED,BAD_CLIENT_STATE);
        cs->retries=0;
        err = Mjpegd_SendData(pcb,cs);//try send more data
        throwif(err != ERR_OK,ERR_SEND);
    }
    catch(BAD_CLIENT_NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("sent BAD_CLIENT_NULL\n"));
        err = ERR_ARG;
    }
    catch(BAD_CLIENT_STATE)
    {
        if(cs->conn_state==CS_CLOSING)
            err = ERR_CLSD;
        else
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                MJPEGD_DBG_ARG("sent 0x%p BAD_CLIENT_STATE %d\n",cs,cs->conn_state));
        }
    }
    catch(ERR_SEND)
    {
        if(err==ERR_MEM)
        {
            //ERR_MEM happen quite often when streaming
            //It'll retry in poll when tcp_sndbuf is available
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
                MJPEGD_DBG_ARG("sent ERR_SEND out of ram\n"));
        }
        else
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                MJPEGD_DBG_ARG("sent cs 0x%p ERR_SEND :%s\n",cs,lwip_strerr(err)));
        }
    }
    finally
    {
        return err;
    }
}

static err_t Mjpegd_LwipPoll_Handler(void *arg, struct tcp_pcb *pcb)
{
    err_t err;
    ClientState_t *cs = (ClientState_t*)arg;
    try
    {
        throwif(cs == NULL,NULL_CLIENT);
        throwif(pcb == NULL,NULL_PCB);
        throwif(cs->conn_state == CS_CLOSING,CONN_CLOSING);
        throwif(cs->conn_state == CS_TCP_ERR,CONN_TCP_ERR);
        throwif(++(cs->retries) > MJPEGD_MAX_RETRIES,CLIENT_TIMEOUT);

        // if(cs->conn_state == CS_RECEIVED)
        // {
            //LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
            //    MJPEGD_DBG_ARG("poll try to send more data\n"));
            //err = Mjpegd_SendData(pcb, cs);

            //throwif(err!=ERR_OK,ERR_SEND_DATA_FAIL);
            
            //tcp_output(pcb);
        // }
        err = ERR_OK;
    }
    catch(NULL_CLIENT)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("poll NULL_CLIENT\n"));
        err = ERR_ARG;
    }
    catch(NULL_PCB)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("poll NULL_PCB\n"));
        err = ERR_ARG;
    }
    catch(CONN_CLOSING)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("poll CONN_CLOSING %p\n",cs ));
        err = ERR_CLSD;
    }
    catch(CONN_TCP_ERR)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("poll CONN_TCP_ERR %p\n",cs ));
        err = ERR_CLSD;
    }
    catch(CLIENT_TIMEOUT)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("poll CLIENT_TIMEOUT %p\n",cs ));        
        err = ERR_TIMEOUT;
    }
    // catch(ERR_SEND_DATA_FAIL)
    // {
    //     LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
    //         MJPEGD_DBG_ARG("poll ERR_SEND_DATA_FAIL %s %s:%d\n", lwip_strerr(err), ipaddr_ntoa(&(pcb->remote_ip)),pcb->remote_port));
    // }
    finally
    {
        if(err!=ERR_OK)
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                MJPEGD_DBG_ARG("poll closing cs %p,pcb %p, %s:%d\n", 
                    cs,pcb,ipaddr_ntoa(&(pcb->remote_ip)),pcb->remote_port));
            Mjpegd_CloseConn(pcb, cs);
        }

        return err;
    }
}

static void Mjpegd_LwipErr_Handler(void *arg, err_t err)
{
    ClientState_t *cs = (ClientState_t*)arg;
    LWIP_UNUSED_ARG(err);

    LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("Mjpegd_LwipErr_Handler:%s\n",lwip_strerr(err)));


    //if err callback triggered
    //pcb may already have been dealloc by lwip core
    //we only dealloc client data here
    if (cs != NULL) 
    {
        Mjpegd_CloseConn(NULL,cs);
    }
}

static err_t Mjpegd_SendData(struct tcp_pcb *pcb, ClientState_t *cs)
{   
    err_t err;
    u32_t left_len=0;
    u16_t w_len=0;
    u8_t flag=0;

    try
    {
        throwif(cs==NULL ,NULL_CS);
        throwif(cs->conn_state!=CS_RECEIVED ,CLIENT_INVALID_STATE);
        throwif(cs->request_handler==NULL ,NULL_REQUEST_HANDLER);        

        //check if we have data left to send
        if(client_file_isvalid(cs))
            left_len = client_file_bytestosend(cs);

        //check EOF and try to get next file
        if(left_len == 0)
        {
            //no more file
            throwif(cs->get_nextfile_func==NULL,EOF_REACHED);
            err = cs->get_nextfile_func(cs);
            //handler request to close connection
            throwif(err==ERR_CLSD,EOF_REACHED);
            //next file not ready
            throwif(err==ERR_INPROGRESS,ERR_GET_NEXTFILE_INPROGRESS);
            //next file failed
            throwif(err!=ERR_OK,ERR_GET_NEXTFILE_FAIL);
            //check again in case something went wrong in get_nextfile_func
            throwif(!client_file_isvalid(cs),BAD_NEXT_FILE);
            left_len = client_file_bytestosend(cs);
        }

        //start sending
        w_len = MJPEGD_MIN(left_len,tcp_sndbuf(pcb));
        w_len = MJPEGD_MIN(w_len,2*tcp_mss(pcb));
        
        do
        {
            if (left_len > w_len)
                flag = TCP_WRITE_FLAG_MORE;

            err = tcp_write(pcb, cs->file_wptr, w_len, flag);

            if (err == ERR_MEM) 
            {   
                //no enough ram, try smaller size
                if ((tcp_sndbuf(pcb) == 0) || (pcb->snd_queuelen >= TCP_SND_QUEUELEN)) 
                    w_len = 0; /* no need to try smaller sizes */
                else 
                    w_len /= 2;
            }
        } while ((err == ERR_MEM) && w_len );

        if(w_len==0) err=ERR_MEM;
        throwif(err != ERR_OK,ERR_TCP_WRITE_ERROR);
        cs->file_wptr += w_len;
        
        //DONT CLOSE CONN HERE!!!
        //in lwip httpd.c example, close_conn is called when reach EOF here
        //but cs->file is mostly point to ram and we dont use TCP_WRITE_FLAG_COPY
        //so we cant close conn(e.g. close file) until receive ack
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("send NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(CLIENT_INVALID_STATE)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("send cs 0x%p CLIENT_INVALID_STATE %d\n",cs,cs->conn_state));
        err=ERR_ARG;
    }
    catch(NULL_REQUEST_HANDLER)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("send cs 0x%p NULL_REQUEST_HANDLER\n",cs));
        err=ERR_ARG;
    }
    catch(ERR_GET_NEXTFILE_INPROGRESS)
    {
        err=ERR_OK; //file in progress, try again later
    }
    catch(ERR_GET_NEXTFILE_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("send GET_NEXTFILE_FAIL %s\n",lwip_strerr(err)));  
    }
    catch(BAD_NEXT_FILE)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            MJPEGD_DBG_ARG("send BAD_NEXT_FILE %p\n",cs->file));
        err=ERR_ARG;
    }
    catch(ERR_TCP_WRITE_ERROR)
    {
        if (err==ERR_MEM)
        {
            //no enough ram, try again later
            //happen quite often when lwip tcp pool usage is high
            //too many streaming clients?
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
               MJPEGD_DBG_ARG("send TCP_WRITE_ERROR out of ram\n"));

            err=ERR_OK;
        }
        else
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                MJPEGD_DBG_ARG("send TCP_WRITE_ERROR %s\n",lwip_strerr(err)));
        }
    }
    catch(EOF_REACHED)
    {
        //EOF reached, no next file, closing conn   
        err=ERR_OK;  
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
            MJPEGD_DBG_ARG("send_data EOF reached, closing %s\n",mjpegd_strreq(cs->request_handler->req)));   
        cs->conn_state = CS_CLOSING;
    }
    finally
    {
        return err;
    }
}

/**
 * @brief Close a client and pcb
 * @warning Do not call during client_list iteration,
 *          This function will remove cs from the list.
 */
static void Mjpegd_CloseConn(struct tcp_pcb *pcb, ClientState_t *cs)
{
    err_t err=ERR_OK;
    
    if(cs!=NULL)
    {
        cs->conn_state = CS_CLOSED;
        if(cs->request_handler != NULL &&
            cs->request_handler->clsd_request_func!=NULL)
            cs->request_handler->clsd_request_func(cs);
        Mjpegd_Client_Free(cs);
        cs=NULL;
    }

    if(pcb!=NULL)
    {
        char ip_str[IP4ADDR_STRLEN_MAX];
        ip4addr_ntoa_r(&(pcb->remote_ip), ip_str, sizeof(ip_str));

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
            MJPEGD_DBG_ARG("CloseConn cs %p, pcb %p %s:%d\n"
                ,cs,pcb,ip_str,pcb->remote_port));

        tcp_arg(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_poll(pcb, NULL, 10);
        tcp_sent(pcb, NULL);
        err = tcp_close(pcb);
    }

    if (err != ERR_OK) 
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            MJPEGD_DBG_ARG("Error %s closing pcb %p\n", 
                lwip_strerr(err) ,pcb));
            /* error closing, try again later in poll */
        tcp_poll(pcb, Mjpegd_LwipPoll_Handler, 10);
    }
}

/**
 * @brief Assign latest frame to stream clients whos frame is empty 
 *        and previous_transfer_time >= fps_period.
 * @note tcp_out is performed in mjpegd_service() loop.
 */
static void Mjpegd_AssignNewframe(Mjpegd_t *mjpegd)
{
    Mjpegd_FramePool_t* frame_pool = mjpegd->FramePool;
    ClientState_t* cs;
    MJPEGD_SYSTIME_T now = sys_now();

    for (cs=mjpegd->_clients_list; cs!=NULL; cs=cs->_next)
    {
        //check if stream client transfer done
        if( cs->conn_state==CS_RECEIVED && cs->frame == NULL && 
            cs->request_handler !=NULL && 
            now - cs->previous_transfer_time >= cs->fps_period &&
           (cs->request_handler->req == REQUEST_STREAM ||
            cs->request_handler->req == REQUEST_SNAP) )
        {
            err_t err;

            try
            {
                cs->frame=Mjpegd_FramePool_GetLatest(frame_pool,cs->previous_frame_time);
                throwif(cs->frame==NULL,GET_FRAME_FAIL);
                throwif(!Mjpegd_Frame_IsValid(cs->frame),BAD_FRAME);
                
                if(cs->request_handler->req == REQUEST_STREAM)
                    client_assign_file(cs,cs->frame->head, Mjpegd_Frame_StreamSize(cs->frame));
                else 
                    client_assign_file(cs,cs->frame->head, Mjpegd_Frame_SnapSize(cs->frame));
                //record frame time
                cs->previous_frame_time = cs->frame->capture_time;
                cs->previous_transfer_time = now;

                err = Mjpegd_SendData(cs->pcb,cs);
                throwif(err!=ERR_OK,ERR_SEND_ERR);
            }
            catch(GET_FRAME_FAIL)
            {
                //frame not available, try again later
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
                    MJPEGD_DBG_ARG("newframe_handler GET_FRAME_FAIL\n"));
            }
            catch(BAD_FRAME)
            {
                //bad frame,release and try again later
                Mjpegd_FramePool_Release(frame_pool,cs->frame);
                cs->frame = NULL;
                
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    MJPEGD_DBG_ARG("newframe_handler BAD_FRAME, try later\n"));
            }
            catch(ERR_SEND_ERR)
            {
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    MJPEGD_DBG_ARG("newframe_handler send %s\n",lwip_strerr(err)));
            }
            finally
            {
                ;
            }
        }
    }
}

//stop camera if no client connected for a while
static void Mjpegd_CheckIdle(Mjpegd_t *mjpegd)
{
    MJPEGD_SYSTIME_T now = sys_now();

    if(mjpegd->_client_count!=0)
        mjpegd->_idle_timer = now;

    if(now - mjpegd->_idle_timer > MJPEGD_IDLE_TIMEOUT)
    {
        //turn off camera if enabled
        if(Mjpegd_Camera_IsEnabled(mjpegd->Camera)==true)
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE, 
                MJPEGD_DBG_ARG("Mjpegd idle, stop camera\n"));
            Mjpegd_Camera_Stop(mjpegd->Camera);
           
            mjpegd->_frame_pool_cleared=0;
        }
        
        //clear frame pool if not cleared yet
        if(mjpegd->_frame_pool_cleared==0)
        {
            mjpegd->_frame_pool_cleared=Mjpegd_FramePool_TryClear(mjpegd->FramePool);
            if(mjpegd->_frame_pool_cleared)
            {
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE, 
                MJPEGD_DBG_ARG("FramePool cleared\n"));
            }
        }
    }
    else
    {
        //turn on camera if disabled
        if(Mjpegd_Camera_IsEnabled(mjpegd->Camera)==false)
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE, 
                MJPEGD_DBG_ARG("Mjpegd active, start camera\n"));
            Mjpegd_Camera_Start(mjpegd->Camera);
        }
        else if(Mjpegd_Camera_IsSnapping(mjpegd->Camera)==false)
        {
            //camera is enabled, but not capturing, start capture
            Mjpegd_Frame_t* frame = Mjpegd_FramePool_GetIdle(mjpegd->FramePool);
            if(frame != NULL)
            {
                if(Mjpegd_Camera_DoSnap(mjpegd->Camera,frame)==false)
                {
                    //failed to start capture, release frame
                    //try again in next loop
                    Mjpegd_FrameProc_RecvBroken(mjpegd,frame);
                    LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                        MJPEGD_DBG_ARG("CheckIdle DoSnap failed\n"));
                }
            }
            else
            {
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                    MJPEGD_DBG_ARG("No idle frame\n"));
            }
        }
    }
}

static void Mjpegd_ShowFps(Mjpegd_t *mjpegd)
{
    MJPEGD_SYSTIME_T now = sys_now();
    if(now - mjpegd->_fps_timer > (1000<<MJPEGD_SHOWFPS_PERIOD))
    {
        MJPEGD_ATOMIC_XCHG(&mjpegd->_fps_timer,now);

        MJPEGD_DBG_PRINT(MJPEGD_DBG_ARG("Mjpegd FPS %d\n",
            (mjpegd->_fps_counter>>MJPEGD_SHOWFPS_PERIOD)));
            
        MJPEGD_ATOMIC_XCHG(&mjpegd->_fps_counter,0);
    }
}

static void Mjpegd_ShowDrop(Mjpegd_t *mjpegd)
{
    u16_t drop_cnt = mjpegd->_drop_counter;

    if(drop_cnt >= MJPEGD_SHOWDROP_THRESHOLD)
    {
        MJPEGD_DBG_PRINT(MJPEGD_DBG_ARG("Frame dropped %d, service too slow?\n"
            ,mjpegd->_drop_counter));
        
        MJPEGD_ATOMIC_XCHG(&mjpegd->_drop_counter,0);
    }
}

/**
 * @brief MJPEGD service loop, regist to lwip timeout to execute periodically   
 * @param arg 
 */
void Mjpegd_Service(void* arg)
{
    Mjpegd_t *mjpegd = (Mjpegd_t*)arg;

    Mjpegd_FrameProc_ProcPending(mjpegd);
    Mjpegd_AssignNewframe(mjpegd);
    Mjpegd_Stream_Output(mjpegd);

    Mjpegd_CheckIdle(mjpegd);
#if MJPEGD_SHOWFPS_PERIOD
    Mjpegd_ShowFps(mjpegd);
#endif
#if MJPEGD_SHOWDROP_THRESHOLD
    Mjpegd_ShowDrop(mjpegd);
#endif

    sys_timeout(MJPEGD_SERVICE_PERIOD, Mjpegd_Service, mjpegd);
}

/**
 * @brief Callback when new frame arrived.
 * @note This function should regist to FRAMEPOOL_CALLBACK_RX_NEWFRAME callback.
 * @param sender source frame_pool.
 * @param arg new arrived frame, not use in this function.
 * @param owner mjpegd instance.
 */
static void Mjpegd_RecvNewFrame_handler(void *sender, void *arg, void *owner)
{
    Mjpegd_AssignNewframe((Mjpegd_t*)owner);
}

