#ifndef __SYSCTRL_H__
#define __SYSCTRL_H__

#include "bsp/platform/platform_defs.h"

#define SysCtrl_EnterSleep  __WFI
#define SysCtrl_IsThreadInIRq() (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk > 0)
#define SysCtrl_Reset   NVIC_SystemReset

void SysCtrl_ResetAfter(uint16_t ms);
void SysCtrl_Service(void);

#endif
