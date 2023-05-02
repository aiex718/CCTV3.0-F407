#ifndef SPI_H
#define SPI_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/rcc.h"
#include "bsp/hal/gpio.h"

typedef struct HAL_SPI_s
{
    //RCC
    HAL_RCC_Cmd_t *SPI_RCC_cmd;
    //SPI
    SPI_TypeDef *SPIx;
    SPI_InitTypeDef *SPI_InitCfg;
    //GPIO
    HAL_GPIO_pin_t *SPI_SckPin;
    HAL_GPIO_pin_t *SPI_MisoPin;
    HAL_GPIO_pin_t *SPI_MosiPin;
    //Null if cs is software controlled, 
    HAL_GPIO_pin_t *SPI_CsPin;
    /* If cs is software controlled, SPI_NSS_Soft must be set 
    in SPI_InitCfg->SPI_NSS.
    
    If cs is hardware controlled, SPI_NSS_Hard must be set
    in SPI_InitCfg->SPI_NSS, remember to disable spi to pull cs 
    high when tx finished.  */
}HAL_SPI_t;

void HAL_SPI_Init(const HAL_SPI_t *self);
void HAL_SPI_Cmd(const HAL_SPI_t *self,bool en);

uint16_t HAL_SPI_Write_Polling(const HAL_SPI_t *self,uint8_t *data,uint16_t len,uint16_t timeout);
uint16_t HAL_SPI_Read_Polling(const HAL_SPI_t *self,uint8_t *data,uint16_t len,uint16_t timeout);
uint16_t HAL_SPI_WriteRead_Polling(const HAL_SPI_t *self,uint8_t *tx_data,uint8_t *rx_data,uint16_t len,uint16_t timeout);




#endif
