#ifndef FLASH_PROG_H
#define FLASH_PROG_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/atomic.h"

typedef struct HAL_FlashProg_s
{
    uint8_t FlashProg_VoltageRange;
}HAL_FlashProg_t;

//This will disable all interrupts
__STATIC_INLINE void HAL_FlashProg_Unlock(HAL_FlashProg_t* self)
{
    Atomic_EnterCritical();
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
}

//This will re-enable all interrupts
__STATIC_INLINE void HAL_FlashProg_Lock(HAL_FlashProg_t* self)
{
    FLASH_Lock();
    Atomic_ExitCritical();
}

__STATIC_INLINE bool HAL_FlashProg_Erase(HAL_FlashProg_t* self,uint16_t Sector)
{
    return FLASH_EraseSector(Sector, self->FlashProg_VoltageRange)==FLASH_COMPLETE;
}

__STATIC_INLINE bool HAL_FlashProg_Write_uint8_t(HAL_FlashProg_t* self,uint32_t addr, uint8_t data)
{
    return FLASH_ProgramByte(addr, data)==FLASH_COMPLETE;
}

__STATIC_INLINE bool HAL_FlashProg_Write_uint16_t(HAL_FlashProg_t* self,uint32_t addr, uint16_t data)
{
    return FLASH_ProgramHalfWord(addr, data)==FLASH_COMPLETE;
}

__STATIC_INLINE bool HAL_FlashProg_Write_uint32_t(HAL_FlashProg_t* self,uint32_t addr, uint32_t data)
{
    return FLASH_ProgramWord(addr, data)==FLASH_COMPLETE;
}

__STATIC_INLINE bool HAL_FlashProg_Write_uint64_t(HAL_FlashProg_t* self,uint32_t addr, uint64_t data)
{
    return FLASH_ProgramDoubleWord(addr, data)==FLASH_COMPLETE;
}

__STATIC_INLINE bool HAL_FlashProg_Write(HAL_FlashProg_t* self,uint32_t addr, void* data, uint16_t len)
{
    uint8_t step;
    bool result = true;
    uint8_t *r_ptr = (uint8_t*)data;
    
    while(len)
    {
        //VoltageRange from 1 to 4 contains value 0 to 3
        step = BSP_MIN(1<<self->FlashProg_VoltageRange, len);

        switch(step)
        {
            case 1:
                result = HAL_FlashProg_Write_uint8_t(self, addr, *(uint8_t*)r_ptr);
                break;
            case 2:
                result = HAL_FlashProg_Write_uint16_t(self, addr, *(uint16_t*)r_ptr);
                break;
            case 4:
                result = HAL_FlashProg_Write_uint32_t(self, addr, *(uint32_t*)r_ptr);
                break;
            case 8:
                result = HAL_FlashProg_Write_uint64_t(self, addr, *(uint64_t*)r_ptr);
                break;
        }

        if(!result)
            return false;
        
        addr += step;
        r_ptr += step;
        len -= step;
    }

    return true;
}

#endif
