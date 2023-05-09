#include "bsp/platform/periph/peri_flash_prog.h"

HAL_FlashProg_t Peri_FlashProg_Inst = {
    .FlashProg_VoltageRange = VoltageRange_3,
};
HAL_FlashProg_t* Peri_FlashProg = &Peri_FlashProg_Inst;
