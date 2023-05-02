#ifndef DISK_H
#define DISK_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/dbg_serial.h"

typedef struct Disk_s
{
    uint8_t lun_id;
    uint8_t is_ready;
    uint8_t is_write_protected;
    uint32_t block_num;
    uint32_t block_size;

    uint32_t driv_id;
    void* p_driv;

    //function pointer
    bool (*Init)(struct Disk_s *self);
    bool (*Read)(struct Disk_s *self,uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
    bool (*Write)(struct Disk_s *self,uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
}Disk_t;

__STATIC_INLINE void Disk_InitAll(Disk_t **list)
{
    uint8_t i=0;

    while(list[i])
    {
        bool result;
        result = list[i]->Init(list[i]);
        list[i]->lun_id = i;

        if(result)
			DBG_INFO("Disk %d init OK\n",i);
		else
			DBG_INFO("Disk %d init failed\n",i);

        i++;
    }
}

__STATIC_INLINE uint8_t Disk_GetNums(Disk_t **list)
{
    uint8_t i=0;
    while(list[i])
        i++;
        
    return i;
}

__STATIC_INLINE void Disk_GetCapacity(Disk_t *self,uint32_t * block_num, uint32_t * block_size)
{
    *block_num = self->block_num;
    *block_size = self->block_size;
}

__STATIC_INLINE bool Disk_IsReady(Disk_t *self)
{ return self->is_ready; }

__STATIC_INLINE bool Disk_IsWriteProtected(Disk_t *self)
{ return self->is_write_protected; }

#endif
