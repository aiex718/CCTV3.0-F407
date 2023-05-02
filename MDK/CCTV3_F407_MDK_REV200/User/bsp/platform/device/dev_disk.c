#include "dev_disk.h"

#include "device/flash_w25qx.h"
#include "device/disk_flash.h"

Disk_t Dev_Disk_Flash_Inst = 
{
    .lun_id = 0,

    .is_ready = false,
    .is_write_protected = false,
    .block_num = FLASH_W25QX_BLOCK_NUM,
    .block_size = FLASH_W25QX_BLOCK_SIZE,
    .driv_id = FLASH_W25QX_JEDEC_ID,

    .Init = Disk_Flash_Init,
    .Read = Disk_Flash_Read,
    .Write = Disk_Flash_Write,

    .p_driv = (void*)__CONST_CAST_VAR(Flash_W25Qx_t){
        .hal_spi = __CONST_CAST_VAR(HAL_SPI_t){
            .SPI_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
                .RCC_APB1Periph = RCC_APB1Periph_SPI3,
            },
            .SPIx = SPI3,
            .SPI_InitCfg = __CONST_CAST_VAR(SPI_InitTypeDef){
                .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
                .SPI_Mode = SPI_Mode_Master,
                .SPI_DataSize = SPI_DataSize_8b,
                .SPI_CPOL = SPI_CPOL_High,
                .SPI_CPHA = SPI_CPHA_2Edge,
                .SPI_NSS = SPI_NSS_Soft,
                .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
                .SPI_FirstBit = SPI_FirstBit_MSB,
                .SPI_CRCPolynomial = 7
            },
            .SPI_SckPin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
                .GPIOx = GPIOC,
                .GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
                    .RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
                },
                .GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
                    .GPIO_Pin = GPIO_Pin_10,
                    .GPIO_Mode = GPIO_Mode_AF,
                    .GPIO_Speed = GPIO_Speed_100MHz,
                    .GPIO_OType = GPIO_OType_PP,
                    .GPIO_PuPd = GPIO_PuPd_NOPULL
                },
                .GPIO_AF_PinSource = GPIO_PinSource10,
                .GPIO_AF_Mapping = GPIO_AF_SPI3,
            },//SPI_SckPin
            .SPI_MisoPin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
                .GPIOx = GPIOC,
                .GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
                    .RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
                },
                .GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
                    .GPIO_Pin = GPIO_Pin_11,
                    .GPIO_Mode = GPIO_Mode_AF,
                    .GPIO_Speed = GPIO_Speed_100MHz,
                    .GPIO_OType = GPIO_OType_PP,
                    .GPIO_PuPd = GPIO_PuPd_NOPULL
                },
                .GPIO_AF_PinSource = GPIO_PinSource11,
                .GPIO_AF_Mapping = GPIO_AF_SPI3,
            },//SPI_MisoPin
            .SPI_MosiPin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
                .GPIOx = GPIOC,
                .GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
                    .RCC_AHB1Periph = RCC_AHB1Periph_GPIOC,
                },
                .GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
                    .GPIO_Pin = GPIO_Pin_12,
                    .GPIO_Mode = GPIO_Mode_AF,
                    .GPIO_Speed = GPIO_Speed_100MHz,
                    .GPIO_OType = GPIO_OType_PP,
                    .GPIO_PuPd = GPIO_PuPd_NOPULL
                },
                .GPIO_AF_PinSource = GPIO_PinSource12,
                .GPIO_AF_Mapping = GPIO_AF_SPI3,
            },
        },//hal_spi
        .cs_pin = __CONST_CAST_VAR(HAL_GPIO_pin_t){
            .GPIOx = GPIOA,
            .GPIO_RCC_cmd = __CONST_CAST_VAR(HAL_RCC_Cmd_t){
                .RCC_AHB1Periph = RCC_AHB1Periph_GPIOA,
            },
            .GPIO_InitCfg = __CONST_CAST_VAR(GPIO_InitTypeDef){		
                .GPIO_Pin = GPIO_Pin_15,
                .GPIO_Mode = GPIO_Mode_OUT,
                .GPIO_Speed = GPIO_Speed_100MHz,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd = GPIO_PuPd_NOPULL
            },
            .GPIO_AF_PinSource = 0,
            .GPIO_AF_Mapping = 0,
        }//cs_pin
    },//p_driv
};

Disk_t *Dev_Disk_Flash = &Dev_Disk_Flash_Inst;




Disk_t *Dev_Disk_list_Inst[] = {
    &Dev_Disk_Flash_Inst,
    NULL
};

Disk_t **Dev_Disk_list = Dev_Disk_list_Inst;
