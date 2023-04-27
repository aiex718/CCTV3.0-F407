#include "app_nettime.h"

NetTime_t App_NetTime_Inst = {
	.NTP_Server = NTP_DEFAULT_SERVER,
};

NetTime_t* App_NetTime = &App_NetTime_Inst;
