#include "bsp/platform/app/app_webhook.h"

Webhook_t Test_Webhook_Inst = {
    .Webhook_Enable = true,
    .Webhook_ConnSetting = NULL, //use default setting
    .Webhook_Host = "httpbin.org",
    .Webhook_Uri = "/get",
    .Webhook_Port = 80,
    .Webhook_Retrys = 5,
    .Webhook_Retry_Delay = 3000,
};

Webhook_t* Test_Webhook = &Test_Webhook_Inst;
