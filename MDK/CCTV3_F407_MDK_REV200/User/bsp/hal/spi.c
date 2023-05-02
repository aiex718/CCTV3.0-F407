#include "bsp/hal/spi.h"

#include "bsp/sys/systimer.h"

__STATIC_INLINE bool HAL_SPI_SendByte(const HAL_SPI_t *self,uint8_t tx,uint8_t* rx_out,uint16_t timeout)
{
    SysTimer_t tmr;
    SPI_TypeDef* spi = self->SPIx;
    
    if(SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == SET)
        SPI_I2S_ReceiveData(spi);//clear previous rx data
    
    SysTimer_Init(&tmr,timeout);

    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET)
    {
        if(SysTimer_IsElapsed(&tmr)) 
            return false;
    }

    SysTimer_Init(&tmr,timeout);
    SPI_I2S_SendData(spi, tx);

    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if(SysTimer_IsElapsed(&tmr)) 
            return false;
    }

    *rx_out=(uint8_t)SPI_I2S_ReceiveData(spi);

    return true;
}

__STATIC_INLINE bool HAL_SPI_WaitBusy(const HAL_SPI_t *self,uint16_t timeout)
{
    SysTimer_t tmr;
    SysTimer_Init(&tmr,timeout);
    while (SPI_I2S_GetFlagStatus(self->SPIx, SPI_I2S_FLAG_BSY) == SET)
    {
        if(SysTimer_IsElapsed(&tmr)) 
            return false;
    }

    return true;
}

void HAL_SPI_Init(const HAL_SPI_t *self)
{
    HAL_RCC_Cmd(self->SPI_RCC_cmd,true);
    SPI_Init(self->SPIx,self->SPI_InitCfg);
    HAL_GPIO_InitPin(self->SPI_SckPin);
    HAL_GPIO_InitPin(self->SPI_MisoPin);
    HAL_GPIO_InitPin(self->SPI_MosiPin);

    if(self->SPI_CsPin)
    {
        HAL_GPIO_InitPin(self->SPI_CsPin);
        SPI_SSOutputCmd(self->SPIx,ENABLE);
    }
}

void HAL_SPI_Cmd(const HAL_SPI_t *self,bool en)
{
    SPI_Cmd(self->SPIx,en?ENABLE:DISABLE);
}

uint16_t HAL_SPI_Write_Polling(const HAL_SPI_t *self,uint8_t *data,uint16_t len,uint16_t timeout)
{
    uint8_t *w_ptr = data;
    uint8_t rx_dummy;

    if(len==0 || data==NULL)
        return 0;

    while (len--)
    {
        if(HAL_SPI_SendByte(self,*w_ptr,&rx_dummy,timeout)==false)
            break;

        w_ptr++;
    }

    HAL_SPI_WaitBusy(self,timeout);

    return (uint16_t)(w_ptr - data);
}

uint16_t HAL_SPI_Read_Polling(const HAL_SPI_t *self,uint8_t *data,uint16_t len,uint16_t timeout)
{
    uint8_t *r_ptr = data;

    if(len==0 || data==NULL)
        return 0;

    while (len--)
    {
        if(HAL_SPI_SendByte(self,0,r_ptr,timeout)==false)
            break;

        r_ptr++;
    }

    HAL_SPI_WaitBusy(self,timeout);

    return (uint16_t)(r_ptr - data);
}

uint16_t HAL_SPI_WriteRead_Polling(const HAL_SPI_t *self,uint8_t *tx_data,uint8_t *rx_data,uint16_t len,uint16_t timeout)
{
    uint8_t *w_ptr = tx_data;
    uint8_t *r_ptr = rx_data;

    if(len==0 || tx_data==NULL || rx_data==NULL)
        return 0;

    while (len--)
    {
        if(HAL_SPI_SendByte(self,*w_ptr,r_ptr,timeout)==false)
            break;

        w_ptr++;
        r_ptr++;
    }

    HAL_SPI_WaitBusy(self,timeout);

    return (uint16_t)(w_ptr - tx_data);
}

