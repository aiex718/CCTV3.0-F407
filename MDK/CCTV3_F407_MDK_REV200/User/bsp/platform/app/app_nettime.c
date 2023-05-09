#include "app_nettime.h"

NetTime_t App_NetTime_Inst = {
	.NetTime_Enable = true,
	.NetTime_SNTP_Server = "time.stdtime.gov.tw",
};

NetTime_t* App_NetTime = &App_NetTime_Inst;
