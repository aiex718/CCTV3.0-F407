#include "device/disk_flash.h"

#include "bsp/sys/dbg_serial.h"
#include "device/flash_w25qx.h"

bool Disk_Flash_Init(Disk_t *self)
{
    Flash_W25Qx_t *drv_w25Qx = (Flash_W25Qx_t *)self->p_driv;
    Flash_W25Qx_Status_t SpiStatus;
    uint32_t id;

    if(self->is_ready==false)
    {
        SpiStatus = Flash_W25Qx_Init(drv_w25Qx);  
        if(SpiStatus != FLASH_W25QX_OK)
        {
            DBG_ERROR("Flash_W25Qx_Init error %d\n",SpiStatus);
            return false;
        }

        SpiStatus = Flash_W25Qx_GetJedecId(drv_w25Qx,&id);  
        
        if (SpiStatus != FLASH_W25QX_OK || id!=self->driv_id)
        {
            DBG_ERROR("Init failed, GetJedecId failed %d, get %x, expect %x\n"
                ,SpiStatus,id,self->driv_id);
            return false;
        }

        self->is_ready = true;
        self->is_write_protected = false;
    }

    return self->is_ready;
}

bool Disk_Flash_Read(Disk_t *self,uint8_t *buf, uint32_t addr, uint16_t len)
{
    Flash_W25Qx_t *drv_w25Qx = (Flash_W25Qx_t *)self->p_driv;
    Flash_W25Qx_Status_t result;

    result = Flash_W25Qx_Read(drv_w25Qx, addr, len, buf);

    if (result != FLASH_W25QX_OK)
        return false;

    return true;
}

bool Disk_Flash_Write(Disk_t *self,uint8_t *buf, uint32_t addr, uint16_t len)
{
    Flash_W25Qx_t *drv_w25Qx = (Flash_W25Qx_t *)self->p_driv;
    Flash_W25Qx_Status_t result;

    if(self->is_write_protected)
        return false;

    //check if addr and len is sector aligned
    if((addr & (FLASH_W25QX_SECTOR_SIZE-1))==0 && (len & (FLASH_W25QX_SECTOR_SIZE-1))==0)
    {
        result = Flash_W25Qx_EraseRange_4K(drv_w25Qx, addr, len);
        if (result != FLASH_W25QX_OK)
            return false;
            
        result = Flash_W25Qx_Write(drv_w25Qx, addr, len , buf);
        if (result != FLASH_W25QX_OK)
            return false;
    }
    else
    {
#if FLASH_W25QX_BLOCK_DIV == 1
        DBG_WARNING("Flash_Write not aligned addr %x,len %x\n",addr,len);
#endif
        result = Flash_W25Qx_Write_AsEEPROM(drv_w25Qx, addr, len, buf);
        if (result != FLASH_W25QX_OK)
            return false;
    }

    return true;
}

