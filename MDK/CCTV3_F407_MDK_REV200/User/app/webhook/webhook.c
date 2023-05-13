#include "app/webhook/webhook.h"

#include "lwip/debug.h"
#include "lwip/timeouts.h"

static void Webhook_GetFile(Webhook_t *self);
static void Webhook_Retry(Webhook_t *self);
static err_t Webhook_HTTPC_RecvHeader_Callback(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len);
static err_t Webhook_HTTPC_Recv_Callback(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err);
static void Webhook_HTTPC_Result_Callback(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err);


const httpc_connection_t Webhook_Default_ConnSetting= {
    .use_proxy = 0,
    .headers_done_fn = Webhook_HTTPC_RecvHeader_Callback,
    .result_fn = Webhook_HTTPC_Result_Callback,
};

void Webhook_ConfigSet(Webhook_t *self,const Webhook_ConfigFile_t *config)
{
    self->Webhook_ConnSetting = config->Webhook_ConnSetting;
    self->Webhook_Enable = config->Webhook_Enable;
    self->Webhook_Retrys = config->Webhook_Retrys;
    self->Webhook_Retry_Delay = config->Webhook_Retry_Delay;
    self->Webhook_Port = config->Webhook_Port;
    self->Webhook_Host = config->Webhook_Host;
    self->Webhook_Uri = config->Webhook_Uri;
}

void Webhook_ConfigExport(const Webhook_t *self,Webhook_ConfigFile_t *config)
{
    config->Webhook_ConnSetting = self->Webhook_ConnSetting;
    config->Webhook_Enable = self->Webhook_Enable;
    config->Webhook_Retrys = self->Webhook_Retrys;
    config->Webhook_Retry_Delay = self->Webhook_Retry_Delay;
    config->Webhook_Port = self->Webhook_Port;
    BSP_STRNCPY(config->Webhook_Host,self->Webhook_Host,sizeof(config->Webhook_Host));
    BSP_ARR_STREND(config->Webhook_Host);
    BSP_STRNCPY(config->Webhook_Uri,self->Webhook_Uri,sizeof(config->Webhook_Uri));
    BSP_ARR_STREND(config->Webhook_Uri);
}

bool Webhook_IsConfigValid(Webhook_t *self,const Webhook_ConfigFile_t *config)
{
    if(config==NULL || config->Webhook_Enable>1)
        return false;
    else if(config->Webhook_Enable == false)
        return true;
    else 
        return  config->Webhook_Retrys  <= 10 && config->Webhook_Port != 0 &&
                config->Webhook_Retry_Delay != 0 && config->Webhook_Retry_Delay < 60000 &&
                config->Webhook_Host[0] != 0 && config->Webhook_Host[0] != 0xff &&
                BSP_STRLEN(config->Webhook_Host) < sizeof(config->Webhook_Host) &&
                config->Webhook_Uri[0] != 0 && config->Webhook_Uri[0] != 0xff && 
                BSP_STRLEN(config->Webhook_Uri) < sizeof(config->Webhook_Uri);

}

void Webhook_Send(Webhook_t *self)
{
    if(self->Webhook_Enable == false)
    {
        LWIP_DEBUGF( WEBHOOK_DEBUG | LWIP_DBG_LEVEL_WARNING, 
                ("Webhook 0x%p send but disabled\n",self));
    }
    else if(Webhook_IsBusy(self))
    {
        LWIP_DEBUGF( WEBHOOK_DEBUG | LWIP_DBG_LEVEL_WARNING, 
                ("Webhook 0x%p err already sending\n",self));
        return;       
    }

    if(self->Webhook_ConnSetting == NULL)
        self->Webhook_ConnSetting = &Webhook_Default_ConnSetting;

    self->webhook_retrys_left = self->Webhook_Retrys;
    self->webhook_httpc_state = NULL;

    Webhook_GetFile(self);
}

static void Webhook_GetFile(Webhook_t *self)
{
    err_t result;

    result = httpc_get_file_dns(self->Webhook_Host, self->Webhook_Port, 
        self->Webhook_Uri, self->Webhook_ConnSetting,
        Webhook_HTTPC_Recv_Callback,self,&self->webhook_httpc_state);

    if(result != ERR_OK)
    {
        LWIP_DEBUGF( WEBHOOK_DEBUG | LWIP_DBG_LEVEL_SERIOUS, 
                ("Send failed %s\n",lwip_strerr(result)));
        Webhook_Retry(self);
    }
}

static err_t Webhook_HTTPC_RecvHeader_Callback(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    //here's the only place we can read header,
    //return ERR_OK to continue, or else to abort
    return ERR_OK;
}

static err_t Webhook_HTTPC_Recv_Callback(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err)
{
    Webhook_t *self = (Webhook_t*)arg;
    if(err != ERR_OK || (p == NULL))
    {
        LWIP_DEBUGF( WEBHOOK_DEBUG | LWIP_DBG_LEVEL_WARNING, 
                ("Webhook0x%p HTTPC_Recv err %s, pbuf 0x%p ,hs 0x%p\n",
                    self,lwip_strerr(err),p,self->webhook_httpc_state));
    }

    //we dont buffer any data, just free it
    if (p != NULL) 
    {
        altcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    }

    return ERR_OK;
}

static void Webhook_HTTPC_Result_Callback(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    Webhook_t *self = (Webhook_t*)arg;
    LWIP_DEBUGF(WEBHOOK_DEBUG | LWIP_DBG_TRACE,
        ("Webhook 0x%p result %d, content len %d ,code %d ,err %d\n",
            self,httpc_result,rx_content_len,srv_res,err));

    if(httpc_result!=HTTPC_RESULT_OK)
    {
        LWIP_DEBUGF(WEBHOOK_DEBUG | LWIP_DBG_LEVEL_WARNING,
            ("Webhook 0x%p failed %d, code %d, err %d, http://%s:%d%s\n",
                self,httpc_result,srv_res,err,
                    self->Webhook_Host,self->Webhook_Port,self->Webhook_Uri));

        Webhook_Retry(self);
    }
    else
    {
        LWIP_DEBUGF( WEBHOOK_DEBUG | LWIP_DBG_STATE, 
                ("Webhook 0x%p success send http://%s:%d%s code %d\n",
                    self,self->Webhook_Host,self->Webhook_Port,self->Webhook_Uri,srv_res));

        self->webhook_httpc_state = NULL;
    }
}

static void Webhook_Retry(Webhook_t *self)
{
    if(self->webhook_retrys_left)
    {
        self->webhook_retrys_left--;
        LWIP_DEBUGF(WEBHOOK_DEBUG | LWIP_DBG_LEVEL_WARNING,
            ("Webhook 0x%p retry left %d in %d ms\n",
                self,self->webhook_retrys_left,self->Webhook_Retry_Delay));
        
        sys_timeout(self->Webhook_Retry_Delay,(sys_timeout_handler)Webhook_GetFile,self);
    }
    else
    {
        LWIP_DEBUGF(WEBHOOK_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            ("Webhook 0x%p aborted http://%s:%d%s\n",
                self,self->Webhook_Host,self->Webhook_Port,self->Webhook_Uri));

        self->webhook_httpc_state = NULL;
    }
}
