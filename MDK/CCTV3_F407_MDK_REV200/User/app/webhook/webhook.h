#ifndef WEBHOOK_H
#define WEBHOOK_H

#include "bsp/platform/platform_defs.h"

#include "lwip/opt.h"

#ifndef WEBHOOK_DEBUG
    #define WEBHOOK_DEBUG 0
#endif

typedef struct Webhook_ConfigFile_s
{
    u8_t Webhook_Enable;
    u8_t Webhook_Retrys;
    u16_t Webhook_Port;
    char Webhook_Host[64];
    char Webhook_Uri[64];
}Webhook_ConfigFile_t;


typedef struct Webhook_s
{
    httpc_connection_t* Webhook_ConnSetting;
    const char* Webhook_Host;
    const char* Webhook_Uri;
    bool Webhook_Enable;
    u8_t Webhook_Retrys;//how many times to retry when failed
    u16_t Webhook_Port;
    //private
    httpc_state_t *webhook_client_state;
    u8_t webhook_retrys_left;
}Webhook_t;

void Webhook_Init(Webhook_t *self);

void Webhook_ConfigSet(Webhook_t *self,const Webhook_ConfigFile_t *config);
void Webhook_ConfigExport(const Webhook_t *self,Webhook_ConfigFile_t *config);
bool Webhook_IsConfigValid(Webhook_t *self,const Webhook_ConfigFile_t *config);

bool Webhook_Send(Webhook_t *self);

#endif
