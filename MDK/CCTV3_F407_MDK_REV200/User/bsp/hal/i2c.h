#ifndef I2C_H
#define I2C_H

#include "bsp/platform/platform_defs.h"
#include "bsp/hal/gpio.h"
#include "bsp/hal/rcc.h"

typedef enum 
{
    HAL_I2C_OK = 0,
    HAL_I2C_BUSY,
    HAL_I2C_ERR_EV5,
    HAL_I2C_ERR_EV6,
    HAL_I2C_ERR_EV7,
    HAL_I2C_ERR_EV8,
    HAL_I2C_ERR_EV8_2,
    HAL_I2C_ERR_AF,//Acknowledge failure
}HAL_I2C_Status_t;

typedef struct HAL_I2C_s
{
    I2C_TypeDef* I2Cx;
    I2C_InitTypeDef *I2C_InitCfg;
    HAL_RCC_Cmd_t *I2C_RCC_Cmd;
    HAL_GPIO_pin_t *I2C_SCL_pin,*I2C_SDA_pin;
}HAL_I2C_t;

#define HAL_I2C_Cmd(i2c,en) I2C_Cmd((i2c)->I2Cx,(en)?ENABLE:DISABLE)

void HAL_I2C_Init(HAL_I2C_t *self);
HAL_I2C_Status_t HAL_I2C_Write_Polling(HAL_I2C_t *self, uint8_t i2c_addr,const uint8_t* data, uint16_t len,uint16_t timeout);
HAL_I2C_Status_t HAL_I2C_Read_Polling(HAL_I2C_t *self, uint8_t i2c_addr, uint8_t* data, uint16_t len,uint16_t timeout);
HAL_I2C_Status_t HAL_I2C_CheckSlaveAck(HAL_I2C_t *self, uint8_t i2c_addr,bool is_reading,uint16_t timeout);


#endif
