#ifndef __DISK_FLASH_H__
#define __DISK_FLASH_H__

#include "bsp/platform/platform_defs.h"

#include "device/disk.h"

bool Disk_Flash_Init(Disk_t *self);
bool Disk_Flash_Read(Disk_t *self,uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
bool Disk_Flash_Write(Disk_t *self,uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);


#endif
