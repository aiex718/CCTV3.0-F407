#ifndef WEBHOOK_H
#define WEBHOOK_H

#include "bsp/platform/platform_defs.h"

#include "lwip/opt.h"
#include "lwip/apps/http_client.h"

#ifndef WEBHOOK_DEBUG
    #define WEBHOOK_DEBUG 0
#endif

typedef struct Webhook_ConfigFile_s
{
    const httpc_connection_t* Webhook_ConnSetting;
    bool Webhook_Enable;
    u8_t Webhook_Retrys;
    u16_t Webhook_Retry_Delay;
    u16_t Webhook_Port;
    u8_t __padding[2];
    char Webhook_Host[64];
    char Webhook_Uri[64];
}Webhook_ConfigFile_t;


typedef struct Webhook_s
{
    const httpc_connection_t* Webhook_ConnSetting;
    const char* Webhook_Host;
    const char* Webhook_Uri;
    bool Webhook_Enable;
    u8_t Webhook_Retrys;//how many times to retry when failed
    u16_t Webhook_Retry_Delay;
    u16_t Webhook_Port;
    //private
    httpc_state_t *webhook_httpc_state;
    u8_t webhook_retrys_left;
}Webhook_t;


void Webhook_ConfigSet(Webhook_t *self,const Webhook_ConfigFile_t *config);
void Webhook_ConfigExport(const Webhook_t *self,Webhook_ConfigFile_t *config);
bool Webhook_IsConfigValid(Webhook_t *self,const Webhook_ConfigFile_t *config);
void Webhook_Send(Webhook_t *self);

__STATIC_INLINE bool Webhook_IsBusy(Webhook_t *self)
{
    return self->webhook_httpc_state != NULL;
}

__STATIC_INLINE bool Webhook_IsEnabled(Webhook_t *self)
{
    return self->Webhook_Enable;
}

#endif
