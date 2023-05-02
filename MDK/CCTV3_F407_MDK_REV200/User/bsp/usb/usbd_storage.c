/**
 ******************************************************************************
 * @file    usbd_storage_msd.c
 * @author  MCD application Team
 * @version V1.2.1
 * @date    17-March-2018
 * @brief   This file provides the disk operations functions.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                      <http://www.st.com/SLA0044>
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------ */
#include "bsp/platform/platform_defs.h"

#include "usbd_msc_mem.h"
#include "usbd_core.h"

#include "bsp/sys/dbg_serial.h"
#include "bsp/platform/device/dev_disk.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @{
 */

/** @defgroup STORAGE
 * @brief media storage application module
 * @{
 */

/** @defgroup STORAGE_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup STORAGE_Private_Defines
 * @{
 */

/**
 * @}
 */

/** @defgroup STORAGE_Private_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup STORAGE_Private_Variables
 * @{
 */
/* USB Mass storage Standard Inquiry Data */
const int8_t STORAGE_Inquirydata[] = {
    /* LUN 0 */
    0x00,
    0x80,
    0x02,
    0x02,
    (USBD_STD_INQUIRY_LENGTH - 5),
    0x00,
    0x00,
    0x00,
    'S', 'T', 'M', '3', '2', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
    'W', '2', '5', 'Q', 'x', ' ', ' ', ' ', /* Product : 16 Bytes */
    'F', '4', '2', '9', ' ', ' ', ' ', ' ',
    '1', '.', '0', '0', /* Version : 4 Bytes */
};

/**
 * @}
 */

/** @defgroup STORAGE_Private_FunctionPrototypes
 * @{
 */
int8_t STORAGE_Init(uint8_t lun);

int8_t STORAGE_GetCapacity(uint8_t lun,
                           uint32_t * block_num, uint32_t * block_size);

int8_t STORAGE_IsReady(uint8_t lun);

int8_t STORAGE_IsWriteProtected(uint8_t lun);

int8_t STORAGE_Read(uint8_t lun,
                    uint8_t * buf, uint32_t blk_addr, uint16_t blk_len);

int8_t STORAGE_Write(uint8_t lun,
                     uint8_t * buf, uint32_t blk_addr, uint16_t blk_len);

int8_t STORAGE_GetMaxLun(void);

USBD_STORAGE_cb_TypeDef USBD_USBMSC_Disk_fops = {
    STORAGE_Init,
    STORAGE_GetCapacity,
    STORAGE_IsReady,
    STORAGE_IsWriteProtected,
    STORAGE_Read,
    STORAGE_Write,
    STORAGE_GetMaxLun,
    (int8_t *)STORAGE_Inquirydata,
};

USBD_STORAGE_cb_TypeDef *USBD_STORAGE_fops = &USBD_USBMSC_Disk_fops;

__IO uint32_t count = 0;
/**
 * @}
 */

/** @defgroup STORAGE_Private_Functions
 * @{
 */

/**
 * @brief  Initialize the storage medium
 * @param  lun : logical unit number
 * @retval Status
 */

int8_t STORAGE_Init(uint8_t lun)
{
    if(lun>=Disk_GetNums(Dev_Disk_list))
        return USBD_FAIL;
    else 
    {
        Disk_t *disk = Dev_Disk_list[lun];
        if(disk->Init(disk) == false)
            return USBD_FAIL;
    }

    return USBD_OK;
}

/**
 * @brief  return medium capacity and block size
 * @param  lun : logical unit number
 * @param  block_num :  number of physical block
 * @param  block_size : size of a physical block
 * @retval Status
 */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num,
                           uint32_t *block_size)
{
    if(lun>=Disk_GetNums(Dev_Disk_list))
        return USBD_FAIL;
    
    Disk_GetCapacity(Dev_Disk_list[lun], block_num, block_size);
    return USBD_OK;
}

/**
 * @brief  check whether the medium is ready
 * @param  lun : logical unit number
 * @retval Status
 */
int8_t STORAGE_IsReady(uint8_t lun)
{
    if(lun>=Disk_GetNums(Dev_Disk_list))
        return USBD_FAIL;
    else if(Disk_IsReady(Dev_Disk_list[lun])==false)
        return USBD_BUSY;
    
    return USBD_OK;
}

/**
 * @brief  check whether the medium is write-protected
 * @param  lun : logical unit number
 * @retval Status
 */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
    if(lun>=Disk_GetNums(Dev_Disk_list))
        return USBD_FAIL;
    else if(Disk_IsWriteProtected(Dev_Disk_list[lun]))
        return USBD_BUSY;
        
    return USBD_OK;
}

/**
 * @brief  Read data from the medium
 * @param  lun : logical unit number
 * @param  buf : Pointer to the buffer to save data
 * @param  blk_addr :  address of 1st block to be read
 * @param  blk_len : nmber of blocks to be read
 * @retval Status
 */
int8_t STORAGE_Read(uint8_t lun,
                    uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    if(lun>=Disk_GetNums(Dev_Disk_list))
        return USBD_FAIL;
    else
    {
        Disk_t *disk = Dev_Disk_list[lun];
        blk_addr *= Dev_Disk_Flash->block_size;
        blk_len *= Dev_Disk_Flash->block_size;

        if(disk->Read(disk,buf , blk_addr , blk_len) == false)
            return USBD_FAIL;
    }
    return USBD_OK;
}

/**
 * @brief  Write data to the medium
 * @param  lun : logical unit number
 * @param  buf : Pointer to the buffer to write from
 * @param  blk_addr :  address of 1st block to be written
 * @param  blk_len : nmber of blocks to be read
 * @retval Status
 */
int8_t STORAGE_Write(uint8_t lun,
                     uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    if(lun>=Disk_GetNums(Dev_Disk_list))
        return USBD_FAIL;
    else
    {
        Disk_t *disk = Dev_Disk_list[lun];
        blk_addr *= Dev_Disk_Flash->block_size;
        blk_len *= Dev_Disk_Flash->block_size;

        if(disk->Write(disk,buf , blk_addr , blk_len)==false)
            return USBD_FAIL;
    }

    return USBD_OK;
}

/**
 * @brief  Return number of supported logical unit
 * @param  None
 * @retval number of logical unit
 */

int8_t STORAGE_GetMaxLun(void)
{
    return (Disk_GetNums(Dev_Disk_list) - 1);
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
