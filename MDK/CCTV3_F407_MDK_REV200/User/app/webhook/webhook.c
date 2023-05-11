#include "app/webhook/webhook.h"

#include "lwip/apps/http_client.h"
#include "lwip/debug.h"

err_t Webhook_HTTPC_RecvHeader_Callback(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len);
err_t Webhook_HTTPC_RecvBody_Callback(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err);
void Webhook_HTTPC_Result_Callback(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err);


const httpc_connection_t Webhook_Default_ConnSetting= {
    .use_proxy = 0,
    .headers_done_fn = Webhook_HTTPC_RecvHeader_Callback,
    .result_fn = Webhook_HTTPC_Result_Callback,
};


void Webhook_Init(Webhook_t *self)
{
    self->webhook_retrys_left = 0;
    self->webhook_client_state = NULL;
}

void Webhook_ConfigSet(Webhook_t *self,const Webhook_ConfigFile_t *config);
void Webhook_ConfigExport(const Webhook_t *self,Webhook_ConfigFile_t *config);
bool Webhook_IsConfigValid(Webhook_t *self,const Webhook_ConfigFile_t *config);

bool Webhook_Send(Webhook_t *self)
{
    err_t result;

    self->webhook_retrys_left = self->Webhook_Retrys;
    result = httpc_get_file_dns(self->Webhook_Host, self->Webhook_Port, 
        self->Webhook_Uri, self->Webhook_ConnSetting,
        Webhook_HTTPC_RecvBody_Callback,self,&self->webhook_client_state);

    if(result != ERR_OK)
    {
        LWIP_DEBUGF( WEBHOOK_DEBUG | LWIP_DBG_LEVEL_WARNING, 
                MJPEGD_DBG_ARG("Send failed %s\n",lwip_strerr(result)));

        return false;
    }
}

err_t Webhook_HTTPC_RecvHeader_Callback(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    printf("Recv header\n");
    printf("%s",hdr->payload);

    return ERR_OK;
}

err_t Webhook_HTTPC_RecvBody_Callback(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err)
{
    printf("Recv body\n");
    printf("%s",p->payload);

    return ERR_OK;
}

void Webhook_HTTPC_Result_Callback(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    printf("Webhook result %d, content len %d ,code %d ,err %d\n",httpc_result,rx_content_len,srv_res,err);
}
