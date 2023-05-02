#ifndef FLASH_W25QX_H
#define FLASH_W25QX_H

#include "bsp/platform/platform_defs.h"

#include "bsp/hal/spi.h"
#include "bsp/hal/gpio.h"

#ifndef FLASH_W25QX_DEBUG
    #define FLASH_W25QX_DEBUG 0
#endif

#ifndef FLASH_W25QX_MODEL
    #define FLASH_W25QX_MODEL 0
#endif

#ifndef FLASH_W25QX_BLOCK_DIV
    #define FLASH_W25QX_BLOCK_DIV 1
#endif

//fast read is required if SPI master is faster than 50MHZ
#ifndef FLASH_W25QX_FAST_READ
    #define FLASH_W25QX_FAST_READ 0
#endif


#if FLASH_W25QX_MODEL == 128
    #define FLASH_W25QX_JEDEC_ID          0xEF4018
    #define FLASH_W25QX_SECTOR_CNT        0x1000 //4096
    #define FLASH_W25QX_SECTOR_SIZE       0x1000 //4096
    #define FLASH_W25QX_CHIP_ERASE_TIMEOUT    200000
#elif FLASH_W25QX_MODEL == 256
    #define FLASH_W25QX_JEDEC_ID          0xEF4019
    #define FLASH_W25QX_SECTOR_CNT        0x2000 //8192
    #define FLASH_W25QX_SECTOR_SIZE       0x1000 //4096
    #define FLASH_W25QX_CHIP_ERASE_TIMEOUT    400000
#else
    #error W25Q chip model not defined
#endif

#define FLASH_W25QX_PAGE_SIZE             256
#define FLASH_W25QX_PAGEWRITE_TIMEOUT     3
#define FLASH_W25QX_SECTOR_ERASE_TIMEOUT  400
#define FLASH_W25QX_CMD_TIMEOUT           15


#define FLASH_W25QX_STORAGE_TOTAL_SIZE    (FLASH_W25QX_SECTOR_CNT * FLASH_W25QX_SECTOR_SIZE)
#define FLASH_W25QX_BLOCK_SIZE            (FLASH_W25QX_SECTOR_SIZE/FLASH_W25QX_BLOCK_DIV)
#define FLASH_W25QX_BLOCK_NUM             (FLASH_W25QX_STORAGE_TOTAL_SIZE/FLASH_W25QX_BLOCK_SIZE)


typedef enum{
    FLASH_W25QX_OK          = 0 ,
    FLASH_W25QX_ERR             ,
    FLASH_W25QX_ERR_SPI         ,
    FLASH_W25QX_ERR_LEN         ,
    FLASH_W25QX_ERR_BUSY        ,
    FLASH_W25QX_ERR_TIMEOUT     ,
}Flash_W25Qx_Status_t;

typedef struct Flash_W25Qx_s
{
    HAL_SPI_t *hal_spi;
    HAL_GPIO_pin_t *cs_pin;
}Flash_W25Qx_t;

//Public functions
Flash_W25Qx_Status_t Flash_W25Qx_Init(Flash_W25Qx_t *self);
Flash_W25Qx_Status_t Flash_W25Qx_GetJedecId(Flash_W25Qx_t *self,uint32_t *id_out);
//Misc
Flash_W25Qx_Status_t Flash_W25Qx_ReadStatusReg(Flash_W25Qx_t *self,bool cs_ctrl,uint8_t *reg_out);
Flash_W25Qx_Status_t Flash_W25Qx_WaitBusy(Flash_W25Qx_t *self,bool cs_ctrl,uint32_t timeout);

//Control
Flash_W25Qx_Status_t Flash_W25Qx_WriteEnable(Flash_W25Qx_t *self,bool en);
Flash_W25Qx_Status_t Flash_W25Qx_Reset(Flash_W25Qx_t *self);
//Erase
Flash_W25Qx_Status_t Flash_W25Qx_EraseChip(Flash_W25Qx_t *self);
Flash_W25Qx_Status_t Flash_W25Qx_EraseSector_4K(Flash_W25Qx_t *self,uint32_t addr);
Flash_W25Qx_Status_t Flash_W25Qx_EraseRange_4K(Flash_W25Qx_t *self,uint32_t addr,uint16_t len);
Flash_W25Qx_Status_t Flash_W25Qx_IsErased(Flash_W25Qx_t *self,uint32_t addr,uint16_t len);
//Write
Flash_W25Qx_Status_t Flash_W25Qx_Write(Flash_W25Qx_t *self,uint32_t addr,uint16_t len,uint8_t* data);
Flash_W25Qx_Status_t Flash_W25Qx_Write_AsEEPROM(Flash_W25Qx_t *self,uint32_t addr,uint16_t len,uint8_t* data);
//Read
Flash_W25Qx_Status_t Flash_W25Qx_Read(Flash_W25Qx_t *self,uint32_t addr,uint16_t len,uint8_t* data);

//Debug and test
#if FLASH_W25QX_DEBUG

#define FLASH_W25QX_TESTLEN_MAX 4096
#define FLASH_W25QX_TESTLEN_MIN 4
#define FLASH_W25QX_RANDOM(min,max) ((rand()%((max) + 1 - (min))) + (min))
#define DumpBuf_Size 16

Flash_W25Qx_Status_t Flash_W25Qx_DumpSector(Flash_W25Qx_t *self,uint32_t addr);
Flash_W25Qx_Status_t Flash_W25Qx_Test(Flash_W25Qx_t *self);
Flash_W25Qx_Status_t Flash_W25QX_Test_RW(Flash_W25Qx_t *self,uint32_t Len,uint32_t Addr,uint8_t *W_Data,uint8_t *R_Data);
Flash_W25Qx_Status_t Flash_W25QX_Test_RW_EEPROM(Flash_W25Qx_t *self,uint32_t Len,uint32_t Addr,uint8_t *W_Data,uint8_t *R_Data);
#endif

#endif
