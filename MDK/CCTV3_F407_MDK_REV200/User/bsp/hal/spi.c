#include "bsp/hal/spi.h"
#include "bsp/sys/systimer.h"

//TODO: Test spi all
//private functions
static HAL_SPI_Status_t __SPI_SendByteRecv(HAL_SPI_t *self,uint8_t tx,uint8_t* rx,uint16_t timeout)
{
    SysTimer_t timeout_tmr;
    SysTimer_Init(&timeout_tmr,timeout);

    if(SPI_I2S_GetFlagStatus(self->SPIx, SPI_I2S_FLAG_BSY) == SET)
        return HAL_SPI_BUSY;

    if(SPI_I2S_GetFlagStatus(self->SPIx, SPI_I2S_FLAG_RXNE) == SET)
        SPI_I2S_ReceiveData(self->SPIx);//clear previous rx data

    SysTimer_Init(&timeout_tmr,timeout);
    while (SPI_I2S_GetFlagStatus(self->SPIx, SPI_I2S_FLAG_TXE) == RESET)
    {
        if(SysTimer_IsElapsed(&timeout_tmr)) 
            return HAL_SPI_ERR_TX;
    }

    SPI_I2S_SendData(self->SPIx, tx);

    while (SPI_I2S_GetFlagStatus(self->SPIx, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if(SysTimer_IsElapsed(&timeout_tmr)) 
            return HAL_SPI_ERR_RX;
    }

    if(rx!=0)
        *rx=(uint8_t)SPI_I2S_ReceiveData(self->SPIx);
    else
        SPI_I2S_ReceiveData(self->SPIx);

    return HAL_SPI_OK;
}

static bool SPI_WaitBusy(HAL_SPI_t *self,uint16_t timeout)
{
    SysTimer_t timeout_tmr;
    SysTimer_Init(&timeout_tmr,timeout);

    while (SPI_I2S_GetFlagStatus(self->SPIx, SPI_I2S_FLAG_BSY) == SET)
    {
        if(SysTimer_IsElapsed(&timeout_tmr)) 
            return false;
    }
    return true;
}

void HAL_SPI_Init(HAL_SPI_t *self)
{
    HAL_RCC_Cmd(self->SPI_RCC_Cmd,ENABLE);
    HAL_GPIO_InitPin(self->SPI_SCK_pin);
    HAL_GPIO_InitPin(self->SPI_MISO_pin);
    HAL_GPIO_InitPin(self->SPI_MOSI_pin);
    SPI_Init(self->SPIx,self->SPI_InitCfg);
}

HAL_SPI_Status_t HAL_SPI_Write_Polling(HAL_SPI_t* self,uint8_t *data,uint16_t len,uint32_t timeout)
{
    HAL_SPI_Status_t result = HAL_SPI_OK;
    while (len--)
    {
        result = __SPI_SendByteRecv(self->SPIx,*data++,(uint8_t*)0,timeout);
        if(result!=HAL_SPI_OK)
            return result;
    }
    result = SPI_WaitBusy(self->SPIx,timeout)? HAL_SPI_OK:HAL_SPI_TIMEOUT;
    return result;
}

HAL_SPI_Status_t HAL_SPI_Read_Polling(HAL_SPI_t* self,uint8_t *data,uint16_t len,uint32_t timeout)
{
    HAL_SPI_Status_t result = HAL_SPI_OK;
    while (len--)
    {
        result = __SPI_SendByteRecv(self->SPIx,0,data++,timeout);
        if(result!=HAL_SPI_OK)
            return result;
    }
    result = SPI_WaitBusy(self->SPIx,timeout)? HAL_SPI_OK:HAL_SPI_TIMEOUT;
    return result;
}

HAL_SPI_Status_t HAL_SPI_WriteRead_Polling(HAL_SPI_t* self,uint8_t *tx_data,uint8_t *rx_data,uint16_t len,uint32_t timeout)
{
    HAL_SPI_Status_t result = HAL_SPI_OK;
    while (len--)
    {
        result = __SPI_SendByteRecv(self->SPIx,*tx_data++,rx_data++,timeout);
        if(result!=HAL_SPI_OK)
            return result;
    }

    result = SPI_WaitBusy(self->SPIx,timeout)? HAL_SPI_OK:HAL_SPI_TIMEOUT;
    return result;
}
