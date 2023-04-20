#include "eth/apps/mjpeg/mjpegd.h"

#include "string.h"
#include "trycatch_while.h"

#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/debug.h"
#include "lwip/stats.h"

/* BSP */
#include "bsp/sys/systime.h"
#include "bsp/sys/callback.h"

/* Devices */
#include "device/cam_ov2640/cam_ov2640.h"
#include "bsp/platform/periph_list.h"

/** Debug defines **/
#include "eth/apps/mjpeg/mjpegd_debug.h"

/** Sub modules **/
#include "eth/apps/mjpeg/mjpegd_framebuf.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/mjpegd_request.h"
#include "eth/apps/mjpeg/mjpegd_client.h"
#include "eth/apps/mjpeg/mjpegd_snap.h"
#include "eth/apps/mjpeg/mjpegd_stream.h"

/** http strings **/
#include "eth/apps/mjpeg/mjpegd_const_string.c"

/** Parameter defines **/
#define MJPEGD_SERVICE_PERIOD 10//10ms
//poll interval is x*2*TCP_TMR_INTERVAL
#define MJPEGD_POLL_INTERVAL 2 //2*2*250=1000ms
//retry limit is x*(poll interval)
#define MJPEGD_MAX_RETRIES 5 //5x1000ms=5s

#define MJPEGD_MAIN_TCP_PRIO TCP_PRIO_NORMAL
#define MJPEGD_STREAM_TCP_PRIO TCP_PRIO_MIN

#define MIN_REQ_LEN 7


/** Private functions **/

/** Lwip raw api callbacks **/
static err_t mjpegd_accept_handler(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t mjpegd_recv_handler(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t mjpegd_sent_handler(void *arg, struct tcp_pcb *pcb, u16_t len);
static err_t mjpegd_poll_handler(void *arg, struct tcp_pcb *pcb);
static void  mjpegd_err_handler(void *arg, err_t err);

/** Lwip timeout callbacks **/
static void mjpegd_timeout_handler(void* arg);

/** mjpegd functions **/
static void  mjpegd_close_conn(struct tcp_pcb *pcb, client_state_t *cs);
static err_t mjpegd_parse_request(struct tcp_pcb *pcb, client_state_t *cs,struct pbuf *p);
static err_t mjpegd_build_response(client_state_t *cs);
static err_t mjpegd_send_data(struct tcp_pcb *pcb, client_state_t *cs);

/** callbacks **/
static void mjpegd_proc_rawframe_handler(void *sender, void *arg, void *owner);
static void mjpegd_send_newframe_handler(void *sender, void *arg, void *owner);

/** http request handlers **/
const request_handler_t request_handlers[__NOT_REQUEST_MAX]=
{
    {REQUEST_NOTFOUND   ,"/404"         ,Http_Notfound_Response     ,MJPEGD_STRLEN(Http_Notfound_Response)      ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_TOOMANY    ,"/429"         ,HTTP_TooMany_Response      ,MJPEGD_STRLEN(HTTP_TooMany_Response)       ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_HANDSHAKE  ,"/handshake"   ,Http_Handshake_Response    ,MJPEGD_STRLEN(Http_Handshake_Response)     ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_VIEW_SNAP  ,"/view/snap"   ,Http_ViewSnap_Response     ,MJPEGD_STRLEN(Http_ViewSnap_Response)      ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_VIEW_STREAM,"/view/stream" ,Http_ViewStream_Response   ,MJPEGD_STRLEN(Http_ViewStream_Response)    ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_VIEW_FPS   ,"/view/fps"    ,Http_ViewFps_Response      ,MJPEGD_STRLEN(Http_ViewFps_Response)       ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_SNAP       ,"/snap"        ,Http_Snap_Response         ,MJPEGD_STRLEN(Http_Snap_Response)          ,mjpegd_nextframe_snap_start,NULL                           ,NULL                       },
    {REQUEST_STREAM     ,"/stream"      ,Http_Stream_Response       ,MJPEGD_STRLEN(Http_Stream_Response)        ,mjpegd_nextframe_stream    ,mjpegd_stream_recv_request     ,mjpegd_stream_clsd_request },
};

#define MJPEGD_FRAMEBUF_LEN (MJPEGD_TOTAL_CLEINT_LIMIT+2)

//main listen pcb
//TODO: let lwip manage mjpegd_pcb
static struct tcp_pcb *mjpegd_pcb;
//TODO:Collect this
static Callback_t Cam2640_NewFrame_cb,Framebuf_RecvRaw_cb,Framebuf_RecvNew_cb;

Mjpegd_FrameBuf_t *Mjpegd_FrameBuf = (Mjpegd_FrameBuf_t*)&(Mjpegd_FrameBuf_t){
    ._frames_len = MJPEGD_FRAMEBUF_LEN,
    ._frames = (Mjpegd_Frame_t*)&(Mjpegd_Frame_t[MJPEGD_FRAMEBUF_LEN]){
        0
    },
};

void Cam2640_NewFrame_Handler(void *sender, void *arg, void *owner)
{
    Device_CamOV2640_t *cam = (Device_CamOV2640_t*)sender;
    u16_t *frame_len = (u16_t *)arg;
    Mjpegd_Frame_t *frame = (Mjpegd_Frame_t*)cam->pExtension;

    //TODO:need a function to end frame
    if(frame!=NULL)
    {
        frame->capture_time = SysTime_Get();
        frame->payload_len = *frame_len;
        frame->tail = frame->payload+frame->payload_len;
        Mjpegd_FrameBuf_ReturnIdle(Mjpegd_FrameBuf,frame);
    }
    else
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            DBG_ARG("Mjpegd: Cam2640_NewFrame NULL\n"));
    }


    frame = Mjpegd_FrameBuf_GetIdle(Mjpegd_FrameBuf);

    if(frame!=NULL)
    {
        //TODO: need a function to set frame
        Mjpegd_Frame_Clear(frame);
        cam->pExtension = frame;
        cam->CamOV2640_FrameBuf = frame->payload;
        cam->CamOV2640_FrameBuf_Len = MJPEGD_FRAME_PAYLOAD_SPACE;
        Device_CamOV2640_SnapCmd(cam,true);
    }
    else
    {
        cam->pExtension = NULL;
        cam->CamOV2640_FrameBuf = NULL;
        cam->CamOV2640_FrameBuf_Len = 0;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            DBG_ARG("Mjpegd: No idle frame\n"));
    }
}

err_t mjpegd_init(u16_t port)
{
    struct tcp_pcb *pcb;
    err_t err;   

    Mjpegd_FrameBuf_Init(Mjpegd_FrameBuf);

    try
    {
        pcb = tcp_new();
        throwif(pcb == NULL,NEW_PCB_FAIL);

        tcp_setprio(pcb, MJPEGD_MAIN_TCP_PRIO);

        err = tcp_bind(pcb, IP_ADDR_ANY, port);
        throwif(err!=ERR_OK,BIND_FAIL);

        mjpegd_pcb = tcp_listen(pcb);
        throwif(mjpegd_pcb==NULL,LISTEN_FAIL);

        tcp_arg(mjpegd_pcb, mjpegd_pcb);
        tcp_accept(mjpegd_pcb, mjpegd_accept_handler);

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SEVERE ,DBG_ARG("init ok at port %d\n",port));

        //TODO:Set owner
        Cam2640_NewFrame_cb.func = Cam2640_NewFrame_Handler;
        Cam2640_NewFrame_cb.invoke_cfg = INVOKE_IMMEDIATELY;
        Cam2640_NewFrame_cb.owner = NULL;
        Device_CamOV2640_SetCallback(Cam_OV2640,CAMOV2640_CALLBACK_NEWFRAME,&Cam2640_NewFrame_cb);

        //Frame buf callbacks always invoked in service
        Framebuf_RecvRaw_cb.func = mjpegd_proc_rawframe_handler;
        Framebuf_RecvRaw_cb.owner = NULL;
        Mjpegd_FrameBuf_SetCallback(Mjpegd_FrameBuf,FRAMEBUF_CALLBACK_RX_RAWFRAME,&Framebuf_RecvRaw_cb);

        Framebuf_RecvNew_cb.func = mjpegd_send_newframe_handler;
        Framebuf_RecvNew_cb.owner = NULL;
        Mjpegd_FrameBuf_SetCallback(Mjpegd_FrameBuf,FRAMEBUF_CALLBACK_RX_NEWFRAME,&Framebuf_RecvNew_cb);

        //invoke newframe handler to start snap TODO: remove this callback invoke?
        Cam2640_NewFrame_Handler(Cam_OV2640,NULL,NULL);
        //invoke timeout handler to start timeout
        mjpegd_timeout_handler(NULL);
        err=ERR_OK;
    }
    catch(NEW_PCB_FAIL)
    {
        err = ERR_MEM;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , DBG_ARG("new pcb fail\n"));
    }
    catch(BIND_FAIL)
    {
        err = ERR_USE;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , DBG_ARG("pcb bind fail\n"));
    }
    catch(LISTEN_FAIL)
    {
        err = ERR_MEM;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , DBG_ARG("pcb listen fail\n"));
    }
    finally
    {
        if(mjpegd_pcb==NULL && pcb!=NULL)
        {
            memp_free(MEMP_TCP_PCB, pcb);
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , DBG_ARG("init failed, free orig pcb\n"));
        }
    }

    return err;
}

/**
 * @brief MJPEGD service loop, regist to lwip timeout to execute periodically   
 * @param arg 
 */
void mjpegd_timeout_handler(void* arg)
{
    Mjpegd_FrameBuf_Service(Mjpegd_FrameBuf);
    mjpegd_stream_output();
    //TODO: restart capture if it's stopped

    sys_timeout(MJPEGD_SERVICE_PERIOD, mjpegd_timeout_handler, NULL);
}

static err_t mjpegd_accept_handler(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    client_state_t* cs;

    struct tcp_pcb_listen *lpcb = (struct tcp_pcb_listen*)arg;

    try
    {
        throwif(err !=ERR_OK,ERR_ACCEPT_HANDLER_ERROR);
        throwif(lpcb == NULL || newpcb == NULL,FATAL_ERROR_NULL_PCB);
        tcp_accepted(lpcb);//unused for lwip 2.1.3, remain for compatibility
        tcp_setprio(newpcb, MJPEGD_STREAM_TCP_PRIO);

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
            DBG_ARG("http_accepted %p / %p\n", (void*)newpcb, arg));

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
            DBG_ARG("New client %s:%d\n", ipaddr_ntoa(&(newpcb->remote_ip)),newpcb->remote_port));

        cs = mjpegd_new_client(newpcb);
        throwif(cs ==NULL,NEW_CLIENT_ERROR);
        
        cs->conn_state = CS_ACCEPTED;
        tcp_arg(newpcb, cs);
        tcp_recv(newpcb, mjpegd_recv_handler);
        tcp_err(newpcb, mjpegd_err_handler);
        tcp_poll(newpcb, mjpegd_poll_handler, MJPEGD_POLL_INTERVAL);
        tcp_sent(newpcb, mjpegd_sent_handler);
        //tcp_nagle_disable(newpcb);//disable nagle for minimal latency

        err = ERR_OK;
    }
    catch(ERR_ACCEPT_HANDLER_ERROR)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            DBG_ARG("accept ERR_ACCEPT_HANDLER_ERROR\n"));
    }
    catch(FATAL_ERROR_NULL_PCB)
    {
        err = ERR_MEM;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            DBG_ARG("accept Main or accept pcb NULL\n"));
    }
    catch(NEW_CLIENT_ERROR)
    {
        err = ERR_MEM;
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            DBG_ARG("accept New client fail\n"));
    }
    finally
    {
        return err;
    }
}

static err_t mjpegd_recv_handler(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{   
    client_state_t *cs = (client_state_t*) arg;
    
    try
    {
        //recv pcb == null, client close conn
        throwif(cs == NULL,BAD_CLIENT_NULL);
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
                DBG_ARG("Received payload:\n%s",(const char*)p->payload));
        #endif

        err = mjpegd_parse_request(pcb,cs,p);
        throwif(err != ERR_OK,ERR_PARSE_REQUEST_FAIL);

        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE, 
            DBG_ARG("Received request:%s\n",mjpegd_strreq(cs->request_handler->req)));


        if(cs->request_handler->recv_request!=NULL)
        {
            err = cs->request_handler->recv_request(cs);
            if(err==ERR_USE)//too many connection
                cs->request_handler = &request_handlers[REQUEST_TOOMANY];
            else
                throwif(err != ERR_OK,ERR_RECV_REQUEST_CALLBACK_FAIL);
        }
        
        err = mjpegd_build_response(cs);
        throwif(err!=ERR_OK,ERR_INIT_RESPONSE_FAIL);
        
        err = mjpegd_send_data(pcb,cs);
        throwif(err!=ERR_OK,ERR_SEND_DATA);
    }
    catch(CONN_CLOSED)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
            DBG_ARG("recv Client closed %s:%d\n", ipaddr_ntoa(&(pcb->remote_ip)),pcb->remote_port));
        err=ERR_CLSD;
    }
    catch(ERR_CONN_ERROR)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("recv ERR_CONN_ERROR %s\n", lwip_strerr(err)));
    }
    catch(BAD_CLIENT_NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("recv BAD_CLIENT_NULL\n"));
        err=ERR_ARG;
    }
    catch(BAD_CLIENT_MULTI_REQUEST)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("recv BAD_CLIENT_MULTI_REQUEST %s:%d\n", ipaddr_ntoa(&(pcb->remote_ip)),pcb->remote_port));
        err=ERR_OK;
    }
    catch(ERR_PARSE_REQUEST_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("recv ERR_PARSE_REQUEST_FAIL %s\n", lwip_strerr(err)));
    }
    catch(ERR_RECV_REQUEST_CALLBACK_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("recv ERR_RECV_REQUEST_CALLBACK_FAIL %s\n", lwip_strerr(err)));
    }
    catch(ERR_INIT_RESPONSE_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("recv ERR_INIT_RESPONSE_FAIL %s\n", lwip_strerr(err)));
    }
    catch(ERR_SEND_DATA)
    {
        //this is our first time send data, should not fail
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("recv ERR_SEND_DATA_FAIL %s\n", lwip_strerr(err)));
    }
    finally
    {
        if (p != NULL) //we dont store any pbuf 
            pbuf_free(p);

        if(err!=ERR_OK)
            mjpegd_close_conn(pcb,cs);

        return err;
    }
}

static err_t mjpegd_sent_handler(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    //client acked, confirm received data
    //continue send data
    err_t err;
    client_state_t *cs = (client_state_t*) arg;
    LWIP_UNUSED_ARG(len);

    try
    {
        throwif(cs == NULL,BAD_CLIENT_NULL);
        cs->retries=0;
        err = mjpegd_send_data(pcb,cs);//try send more data
        throwif(err != ERR_OK,ERR_SEND);
        cs->previous_transfer_time = SysTime_Get();
    }
    catch(BAD_CLIENT_NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("sent BAD_CLIENT_NULL\n"));
        err = ERR_ARG;
    }
    catch(ERR_SEND)
    {
        if(err==ERR_MEM)
        {
            //ERR_MEM happen quite often when lots stream clients connecting
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
                DBG_ARG("sent ERR_SEND out of ram\n"));
        }
        else
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                DBG_ARG("sent ERR_SEND :%s\n",lwip_strerr(err)));
        }
    }
    finally
    {
        return err;
    }
}

static err_t mjpegd_poll_handler(void *arg, struct tcp_pcb *pcb)
{
    err_t err;
    client_state_t *cs = (client_state_t*)arg;
    try
    {
        throwif(cs == NULL,NULL_CLIENT);
        throwif(++(cs->retries) > MJPEGD_MAX_RETRIES,CLIENT_TIMEOUT);

        // if(cs->conn_state == CS_RECEIVED)
        // {
            //LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
            //    DBG_ARG("poll try to send more data\n"));
            //err = mjpegd_send_data(pcb, cs);

            //throwif(err!=ERR_OK,ERR_SEND_DATA_FAIL);
            
            //tcp_output(pcb);
        // }
        err = ERR_OK;
    }
    catch(NULL_CLIENT)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("poll NULL_CLIENT\n"));
        err = ERR_ARG;
    }
    catch(CLIENT_TIMEOUT)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("poll CLIENT_TIMEOUT %s:%d\n", ipaddr_ntoa(&(pcb->remote_ip)),pcb->remote_port));
        err = ERR_TIMEOUT;
    }
    // catch(ERR_SEND_DATA_FAIL)
    // {
    //     LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
    //         DBG_ARG("poll ERR_SEND_DATA_FAIL %s %s:%d\n", lwip_strerr(err), ipaddr_ntoa(&(pcb->remote_ip)),pcb->remote_port));
    // }
    finally
    {
        if(err!=ERR_OK)
            mjpegd_close_conn(pcb, cs);

        return err;
    }
}

static void mjpegd_err_handler(void *arg, err_t err)
{
    client_state_t *cs = (client_state_t*)arg;
    LWIP_UNUSED_ARG(err);

    LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("mjpegd_err_handler:%s\n",lwip_strerr(err)));


    //if err callback triggered
    //pcb may already have been dealloc by lwip core
    //we only dealloc client data here
    if (cs != NULL) 
    {
        mjpegd_close_conn(NULL,cs);
    }
}

static err_t mjpegd_parse_request(struct tcp_pcb *pcb, client_state_t *cs,struct pbuf *p)
{
    char *data= (char *)p->payload;
    u16_t data_len= p->len;
    err_t err;

    if (p->len != p->tot_len) 
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("incomplete requset\n"));
    }

    try
    {
        char *line_end;
        u8_t i;
        throwif (cs==NULL,NULL_CLIENT);

        line_end=MJPEGD_STRNSTR(data, "\r\n",data_len);
        throwif (data_len < MIN_REQ_LEN || line_end==NULL, REQUEST_NOT_COMPLETE);
        throwif (MJPEGD_STRNCMP(data, "GET ", 4) , NOT_HTTP_GET);

        data+=4;
        cs->request_handler = &request_handlers[REQUEST_NOTFOUND];

        for ( i = 0; i < MJPEGD_ARRLEN(request_handlers); i++)
        {
            const request_handler_t* handler = &request_handlers[i];
            if (!MJPEGD_STRNCMP(data, handler->url, strlen(handler->url)))
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
            DBG_ARG("parse NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(REQUEST_NOT_COMPLETE)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("parse REQUEST_NOT_COMPLETE:%s\n",(char*)p->payload));

        err=ERR_ARG;
    }
    catch(NOT_HTTP_GET)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("parse NOT_HTTP_GET:%s\n",(char*)p->payload));

        err=ERR_ARG;
    }
    finally
    {
        return err;
    }

}

static err_t mjpegd_build_response(client_state_t *cs)
{
    err_t err;

    try
    {
        throwif (cs==NULL,NULL_CS);
        throwif (cs->request_handler==NULL,NULL_REQUEST_HANDLER);

        client_assign_file(cs,
            (u8_t*)cs->request_handler->response,
            cs->request_handler->response_len);

        cs->get_nextfile = cs->request_handler->get_nextfile;

        err=ERR_OK;
    }
    catch(NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("response NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(NULL_REQUEST_HANDLER)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("response NULL_REQUEST_HANDLER\n"));

        err=ERR_ARG;
    }
    finally
    {
        return err;
    }
}

static err_t mjpegd_send_data(struct tcp_pcb *pcb, client_state_t *cs)
{   
    err_t err;
    u32_t left_len=0;
    u16_t w_len=0;
    u8_t flag=0;

    try
    {
        throwif(cs==NULL ,NULL_CS);
        throwif(cs->request_handler==NULL ,NULL_REQUEST_HANDLER);
        throwif(!client_file_isvalid(cs),BAD_FILE);

        //check EOF and next file
        left_len = client_file_bytestosend(cs);
        if(left_len <= 0)
        {
            if(cs->get_nextfile!=NULL)
            {
                //try to get next file
                err = cs->get_nextfile(cs);
                throwif(err==ERR_INPROGRESS ,ERR_GET_NEXTFILE_INPROGRESS);
                throwif(err!=ERR_OK ,ERR_GET_NEXTFILE_FAIL);
                //check again in case something went wrong in get_nextfile
                throwif(!client_file_isvalid(cs),BAD_NEXT_FILE);
                left_len = client_file_bytestosend(cs);
            }
            else
                throw(EOF_REACHED);
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
            DBG_ARG("send NULL_CLIENT\n"));
        err=ERR_ARG;
    }
    catch(NULL_REQUEST_HANDLER)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("send NULL_REQUEST_HANDLER\n"));
        err=ERR_ARG;
    }
    catch(BAD_FILE)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("send BAD_FILE %p\n",cs->file));
        err=ERR_ARG;
    }
    catch(ERR_GET_NEXTFILE_INPROGRESS)
    {
        err=ERR_OK; //file in progress, try again later
    }
    catch(ERR_GET_NEXTFILE_FAIL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("send GET_NEXTFILE_FAIL %s\n",lwip_strerr(err)));  
    }
    catch(BAD_NEXT_FILE)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("send BAD_NEXT_FILE %p\n",cs->file));
        err=ERR_ARG;
    }
    catch(ERR_TCP_WRITE_ERROR)
    {
        if (err==ERR_MEM)
        {
            //no enough ram, try again later
            //LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            //    DBG_ARG("send TCP_WRITE_ERROR out of ram\n"));
        }
        else
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                DBG_ARG("send TCP_WRITE_ERROR %s\n",lwip_strerr(err)));
        }
    }
    catch(EOF_REACHED)
    {
        //EOF reached, no next file, closing conn   
        err=ERR_OK;  
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
            DBG_ARG("send_data EOF reached, closing %s\n",mjpegd_strreq(cs->request_handler->req)));   
        mjpegd_close_conn(pcb,cs);
    }
    finally
    {
        return err;
    }
}

static void mjpegd_close_conn(struct tcp_pcb *pcb, client_state_t *cs)
{
    err_t err=ERR_OK;
    LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
        DBG_ARG("Closing connection %p %s:%d\n",pcb , ipaddr_ntoa(&(pcb->remote_ip)),pcb->remote_port));
    
    if(cs!=NULL)
    {
        cs->conn_state = CS_CLOSING;
        if(cs->request_handler->clsd_request!=NULL)
            cs->request_handler->clsd_request(cs);
    }

    if(pcb!=NULL)
    {
        tcp_arg(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_poll(pcb, NULL, 0);
        tcp_sent(pcb, NULL);
        err = tcp_close(pcb);
    }

    mjpegd_free_client(pcb,cs);

    if (err != ERR_OK) 
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("Error %d closing %p %s:%d\n", lwip_strerr(err) ,pcb, ipaddr_ntoa(&(pcb->remote_ip)),pcb->remote_port));
            /* error closing, try again later in poll */
        tcp_poll(pcb, mjpegd_poll_handler, 0);
    }
}

/**
 * @brief Add headers and HTTP EOF to received frame.
 * @note  This function should regist to FrameBuffer_ReceiveRawFrame_Event.
 * @param frame Received frame
 */
static void mjpegd_proc_rawframe_handler(void *sender, void *arg, void *owner)
{
    Mjpegd_Frame_t* frame = (Mjpegd_Frame_t*)arg;
    u8_t buf[30];
    u16_t w_len;
    
    try
    {
        throwif(frame==NULL,NULL_FRAME);
        throwif(frame->payload_len==0,EMPTY_FRAME);
        throwif(frame->head!=frame->payload,ALREADY_PROCESSED);
        
        //insert jpeg comment section
        //this will overwrite old frame SOI(FF D8) from frame->head
        {
            u8_t* comment_wptr;
            frame->payload -= (Mjpeg_Jpeg_Comment_len-2);
            frame->head -= (Mjpeg_Jpeg_Comment_len-2);
            frame->payload_len +=(Mjpeg_Jpeg_Comment_len-2);
            
            MJPEGD_MEMCPY(frame->payload,Mjpeg_Jpeg_Comment,Mjpeg_Jpeg_Comment_len);
            comment_wptr = frame->head+6;
        
            //TODO:insert RTC time 8 byte
            u8_t FakeRTC[8]={0,1,2,3,4,5,6,7};
            MJPEGD_MEMCPY(comment_wptr,FakeRTC,sizeof(FakeRTC));
            comment_wptr+=8;

            //insert frame time 4byte
            MJPEGD_MEMCPY(comment_wptr,&frame->capture_time,sizeof(frame->capture_time));
            comment_wptr+=sizeof(frame->capture_time);
        }
        //insert mjpeg header
        w_len = sprintf((char*)buf,Http_Mjpeg_ContentLength,frame->payload_len);
        Mjpegd_Frame_WriteHeader(frame,buf,w_len);
        Mjpegd_Frame_WriteHeader(frame,(u8_t*)Http_Mjpeg_ContentType,MJPEGD_STRLEN(Http_Mjpeg_ContentType));
        Mjpegd_Frame_WriteHeader(frame,(u8_t*)Http_Mjpeg_Boundary,MJPEGD_STRLEN(Http_Mjpeg_Boundary));
        w_len = sprintf((char*)buf,"%x\r\n", Mjpegd_Frame_HeaderSize(frame) + frame->payload_len);
        Mjpegd_Frame_WriteHeader(frame,buf,w_len);

        Mjpegd_Frame_WriteTail(frame,(u8_t*)Http_CLRF,MJPEGD_STRLEN(Http_CLRF));
    }
    catch(NULL_FRAME)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            DBG_ARG("RawFrame_Handler NULL_FRAME\n"));
    }
    catch(EMPTY_FRAME)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("RawFrame_Handler EMPTY_FRAME\n"));
    }
    catch(ALREADY_PROCESSED)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
            DBG_ARG("RawFrame_Handler ALREADY_PROCESSED\n"));
    }
    finally
    {
        return;
    }

}

/**
 * @brief Callback when new frame arrived, assign to stream clients whos frame is empty.
 * @note tcp_out is performed in mjpegd_service() loop.
 * @note This function should regist to FrameBuffer_ReceiveNewFrame_Event
 * @warning Current implementation is not thread-safe, do not run this handler
 *          outside lwip tcpip_thread.
 * @param frame new arrived frame, not use in this function.
 */
static void mjpegd_send_newframe_handler(void *sender, void *arg, void *owner)
{
    //Mjpegd_Frame_t* new_frame = (Mjpegd_Frame_t*)arg;
    client_state_t* cs;
    for (cs=mjpegd_get_clients(); cs!=NULL; cs=cs->_next)
    {
        //check if stream client transfer done
        if( cs->conn_state==CS_RECEIVED && 
            cs->request_handler->req == REQUEST_STREAM &&
            cs->frame == NULL)
        {
            err_t err;

            try
            {               
                cs->frame=Mjpegd_FrameBuf_GetLatest(Mjpegd_FrameBuf,cs->previous_frame_time);
                throwif(cs->frame==NULL,GET_FRAME_FAIL);
                throwif(!Mjpegd_Frame_IsValid(cs->frame),BAD_FRAME);

                //record frame time
                client_assign_file(cs,cs->frame->head, Mjpegd_Frame_TotalSize(cs->frame));
                cs->previous_frame_time = cs->frame->capture_time;

                err = mjpegd_send_data(cs->pcb,cs);
                throwif(err!=ERR_OK,ERR_SEND_ERR);
            }
            catch(GET_FRAME_FAIL)
            {
                //frame not available, try again later
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                    DBG_ARG("newframe_handler GET_FRAME_FAIL\n"));
            }
            catch(BAD_FRAME)
            {
                //bad frame,release and try again later
                Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf,cs->frame);
                cs->frame = NULL;
                
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    DBG_ARG("newframe_handler BAD_FRAME, try later\n"));

                err=ERR_INPROGRESS;
            }
            catch(ERR_SEND_ERR)
            {
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    DBG_ARG("newframe_handler send %s\n",lwip_strerr(err)));
            }
            finally
            {
                ;
            }
        }
    }
}

