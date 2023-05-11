#include "bsp/platform/app/app_webhook.h"

Webhook_t Current_Trig_Webhook_Inst = {
    .Webhook_Enable = true,
    .Webhook_ConnSetting = NULL, //use default setting
    .Webhook_Host = "server.com",
    .Webhook_Uri = "/triggered?device_id=1234567890",
    .Webhook_Port = 80,
    .Webhook_Retrys = 3,
    .Webhook_Retry_Delay = 3000,
};

Webhook_t* Current_Trig_Webhook = &Current_Trig_Webhook_Inst;
