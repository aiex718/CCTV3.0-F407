#include "bsp/hal/i2c.h"
#include "bsp/sys/systimer.h"

//TODO: Test i2c all
void HAL_I2C_Init(HAL_I2C_t *self)
{
    HAL_RCC_Cmd(self->I2C_RCC_Cmd,ENABLE);
    HAL_GPIO_InitPin(self->I2C_SCL_pin);
    HAL_GPIO_InitPin(self->I2C_SDA_pin);
    I2C_Init(self->I2Cx,self->I2C_InitCfg);
    I2C_AcknowledgeConfig(self->I2Cx, ENABLE);  
}

HAL_I2C_Status_t HAL_I2C_Write_Polling(HAL_I2C_t *self, uint8_t i2c_addr,const uint8_t* data, uint16_t len,uint16_t timeout)
{
    SysTimer_t _timeoutTimer,*timeout_tmr=&_timeoutTimer;
    HAL_I2C_Status_t result=HAL_I2C_OK;

    Timer_Init(timeout_tmr,timeout);
    //Wait i2c line idle
    while(I2C_GetFlagStatus(self->I2Cx, I2C_FLAG_BUSY))   
    {
        if(SysTimer_IsElapsed(timeout_tmr)) 
        {
            result = HAL_I2C_BUSY; 
            goto Finally;
        } 
    }

    I2C_GenerateSTOP(self->I2Cx, DISABLE);
    I2C_GenerateSTART(self->I2Cx, ENABLE);
    SysTimer_Reset(timeout_tmr);

    //Test on EV5 and clear it
    while(!I2C_CheckEvent(self->I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(SysTimer_IsElapsed(timeout_tmr))
        {
            result = HAL_I2C_ERR_EV5; 
            goto Finally;
        } 
    }    

    //Send address and indicate write bit
    I2C_Send7bitAddress(self->I2Cx, i2c_addr, I2C_Direction_Transmitter);  
    SysTimer_Reset(timeout_tmr);

    //Test on EV6 and clear it
    while(!I2C_CheckEvent(self->I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if(SysTimer_IsElapsed(timeout_tmr)) 
        {
            result = HAL_I2C_ERR_EV6; 
            goto Finally;
        }
    }    

    while (len--)
    {
        I2C_SendData(self->I2Cx, *data++);
        SysTimer_Reset(timeout_tmr);

        //Test on EV8_2 and clear it
        while(!I2C_CheckEvent(self->I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))  
        {
            if(SysTimer_IsElapsed(timeout_tmr)) 
            {
                result = HAL_I2C_ERR_EV8_2;
                goto Finally;
            }
        } 
    }

Finally:
    I2C_GenerateSTOP(self->I2Cx, ENABLE);
    return result;
}

HAL_I2C_Status_t HAL_I2C_Read_Polling(HAL_I2C_t *self, uint8_t i2c_addr, uint8_t* data, uint16_t len,uint16_t timeout)
{
    SysTimer_t _timeoutTimer,*timeout_tmr=&_timeoutTimer;
    HAL_I2C_Status_t result=HAL_I2C_OK;

    Timer_Init(timeout_tmr,timeout);
    //Wait i2c line idle
    while(I2C_GetFlagStatus(self->I2Cx, I2C_FLAG_BUSY))   
    {
        if(SysTimer_IsElapsed(timeout_tmr)) 
        {
            result = HAL_I2C_BUSY; 
            goto Finally;
        } 
    }

    if(len<=1)
        I2C_AcknowledgeConfig(self->I2Cx, DISABLE);    
    else
        I2C_AcknowledgeConfig(self->I2Cx, ENABLE);    

    I2C_GenerateSTOP(self->I2Cx, DISABLE);
    I2C_GenerateSTART(self->I2Cx, ENABLE);
    SysTimer_Reset(timeout_tmr);

    //Test on EV5 and clear it
    while(!I2C_CheckEvent(self->I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(SysTimer_IsElapsed(timeout_tmr))
        {
            result = HAL_I2C_ERR_EV5; 
            goto Finally;
        } 
    }    

    //Send address and indicate read bit
    I2C_Send7bitAddress(self->I2Cx, i2c_addr, I2C_Direction_Receiver);  
    SysTimer_Reset(timeout_tmr);

    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(self->I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if(SysTimer_IsElapsed(timeout_tmr)) 
        {
            result = HAL_I2C_ERR_EV6; 
            goto Finally;
        }
    }    

    while(len--)  
    {
        SysTimer_Reset(timeout_tmr);
        while(I2C_CheckEvent(self->I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)==0)  
        {
            if(SysTimer_IsElapsed(timeout_tmr))
            {
                result = HAL_I2C_ERR_EV7;
                goto Finally;
            }
        } 	
        *data++ = I2C_ReceiveData(self->I2Cx);

        if(len <= 1)
        {
            //Next byte is last byte, dont ack
            I2C_AcknowledgeConfig(self->I2Cx, DISABLE);
        }
    }

Finally:
    I2C_GenerateSTOP(self->I2Cx, ENABLE);
    //Enable Acknowledgement for next reception
    I2C_AcknowledgeConfig(self->I2Cx, ENABLE);
    return result;
}

//Some slave use this method to check busy
HAL_I2C_Status_t HAL_I2C_CheckSlaveAck(HAL_I2C_t *self, uint8_t i2c_addr,bool is_reading,uint16_t timeout)
{
    SysTimer_t _timeoutTimer,*timeout_tmr=&_timeoutTimer;
    HAL_I2C_Status_t result=HAL_I2C_OK;    

    Timer_Init(timeout_tmr,timeout);
    //Wait i2c line idle
    while(I2C_GetFlagStatus(self->I2Cx, I2C_FLAG_BUSY))  
    {
        if(SysTimer_IsElapsed(timeout_tmr)) 
        {
            result = HAL_I2C_BUSY; 
            goto Finally;
        } 
    }

    I2C_GenerateSTART(self->I2Cx, ENABLE);
    SysTimer_Reset(timeout_tmr);

    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(self->I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(SysTimer_IsElapsed(timeout_tmr))
        {
            result = HAL_I2C_ERR_EV5;
            goto Finally;
        } 
    }    

    //Send address and rw bit 
    I2C_Send7bitAddress(self->I2Cx, i2c_addr, 
        is_reading?I2C_Direction_Receiver:I2C_Direction_Transmitter);  
    SysTimer_Reset(timeout_tmr);

    //Wait addr bit or ack failure
    while ( I2C_GetFlagStatus(self->I2Cx,I2C_FLAG_ADDR)==0 &&
            I2C_GetFlagStatus(self->I2Cx,I2C_FLAG_AF)==0 )
    {
        if(SysTimer_IsElapsed(timeout_tmr))
        {
            result = HAL_I2C_ERR_AF;
            goto Finally;
        } 
    }

    result = I2C_GetFlagStatus(self->I2Cx,I2C_FLAG_AF) ? HAL_I2C_ERR_AF : HAL_I2C_OK;

Finally:
    //Clear AF flag
    I2C_ClearFlag(self->I2Cx, I2C_FLAG_AF);
    I2C_GenerateSTOP(self->I2Cx, ENABLE); 

    return result;
}
