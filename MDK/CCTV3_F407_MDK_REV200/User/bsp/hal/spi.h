#ifndef SPI_H
#define SPI_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/rcc.h"

typedef enum 
{
    HAL_SPI_OK = 0,
    HAL_SPI_BUSY,
    HAL_SPI_TIMEOUT,
    HAL_SPI_ERR_TX,
    HAL_SPI_ERR_RX,
}HAL_SPI_Status_t;

typedef struct HAL_SPI_s
{
    SPI_TypeDef *SPIx;
    SPI_InitTypeDef *SPI_InitCfg;
    HAL_RCC_Cmd_t *SPI_RCC_Cmd;
    HAL_GPIO_pin_t *SPI_SCK_pin,*SPI_MISO_pin,*SPI_MOSI_pin;
}HAL_SPI_t;
#define HAL_SPI_Cmd(spi,en) SPI_Cmd((spi)->SPIx,(en)?ENABLE:DISABLE)

void HAL_SPI_Init(HAL_SPI_t *self);
HAL_SPI_Status_t HAL_SPI_Write_Polling(HAL_SPI_t *self,uint8_t *data,uint16_t len,uint32_t timeout);
HAL_SPI_Status_t HAL_SPI_Read_Polling(HAL_SPI_t *self,uint8_t *data,uint16_t len,uint32_t timeout);
HAL_SPI_Status_t HAL_SPI_WriteRead_Polling(HAL_SPI_t *self,uint8_t *tx_data,uint8_t *rx_data,uint16_t len,uint32_t timeout);


#endif
