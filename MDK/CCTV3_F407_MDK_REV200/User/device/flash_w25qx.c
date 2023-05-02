#include "device/flash_w25qx.h"

#include "bsp/sys/dbg_serial.h"
#include "bsp/sys/systimer.h"

//opcodes
enum {
OP_WRITE_ENABLE         =0x06,
OP_WRITE_DISABLE        =0x04,

OP_READ_JEDEC_ID        =0x9F,
OP_READ_STATUS_REG      =0x05,
OP_READ_DATA            =0x03,
OP_READ_DATA_ADDR4      =0x13,

OP_FASTREAD_DATA        =0x0B,
OP_FASTREAD_DATA_ADDR4  =0x0C,

OP_WRITE_DATA           =0x02,
OP_WRITE_DATA_ADDR4     =0x12,

OP_ERASE_SECTOR         =0x20,
OP_ERASE_SECTOR_ADDR4   =0x21,
OP_ERASE_CHIP           =0xC7,

OP_RESET_EN             =0x66,
OP_RESET                =0x99,
};


//helper macro
#define RET_ERR(stat) do{                           \
    if((stat)!=FLASH_W25QX_OK)                      \
    {                                               \
        DBG_ERROR("Flash_W25QX error: %d\n",stat);  \
        return (stat);                              \
    }                                               \
}while(0);

#define IS_LEN_VALID(addr,len) (((addr)+(len))<=W25QX_STORAGE_TOTAL_SIZE && (len)>0)
#define IS_ADDR_PAGE_ALIGN(addr) (!((addr) & (W25QX_PAGE_SIZE-1)))

//mask the address to sector boundary
#define ADDR_TO_SECTOR_NUM(addr) ((addr)&(~(W25QX_SECTOR_SIZE-1)))

//CS pin control
#define CS_LOW(self) FLASH_W25QX_CS_SET((self),false)
#define CS_HIGH(self) FLASH_W25QX_CS_SET((self),true)

#define FLASH_W25QX_CS_SET(self,val) do {       \
    HAL_GPIO_WritePin(self->cs_pin,val);        \
    while(HAL_GPIO_ReadPin(self->cs_pin)!=val); \
}while(0);

__STATIC_INLINE Flash_W25Qx_Status_t __w25qx_lowlevel_write(Flash_W25Qx_t *self,
    uint8_t *buf,uint16_t len,uint16_t timeout)
{
    if(HAL_SPI_Write_Polling(self->hal_spi,buf,len,timeout)==len)
        return FLASH_W25QX_OK;                                      
    else                                                                   
        return FLASH_W25QX_ERR_SPI;  
}

__STATIC_INLINE Flash_W25Qx_Status_t __w25qx_lowlevel_read(Flash_W25Qx_t *self,
    uint8_t *buf,uint16_t len,uint16_t timeout)
{
    if(HAL_SPI_Read_Polling(self->hal_spi,buf,len,timeout)==len)
        return FLASH_W25QX_OK;                                      
    else                                                                   
        return FLASH_W25QX_ERR_SPI;  
}

__STATIC_INLINE Flash_W25Qx_Status_t __w25qx_lowlevel_write_read(Flash_W25Qx_t *self,
    uint8_t *wbuf,uint8_t *rbuf,uint16_t len,uint16_t timeout)
{
    if(HAL_SPI_WriteRead_Polling(self->hal_spi,wbuf,rbuf,len,timeout)==len)
        return FLASH_W25QX_OK;                                      
    else                                                                   
        return FLASH_W25QX_ERR_SPI;  
}



Flash_W25Qx_Status_t Flash_W25Qx_Init(Flash_W25Qx_t *self)
{
    uint32_t id;
    Flash_W25Qx_Status_t SpiStatus;

    HAL_SPI_Init(self->hal_spi);
    HAL_GPIO_InitPin(self->cs_pin);
    HAL_GPIO_WritePin(self->cs_pin,true);
    HAL_SPI_Cmd(self->hal_spi,true);

    //transfer dummy for cpol/cpha correction
    SpiStatus = __w25qx_lowlevel_write(self,(uint8_t*)&id,1,W25QX_CMD_TIMEOUT);
    RET_ERR(SpiStatus);

    SpiStatus = Flash_W25Qx_Reset(self);
    RET_ERR(SpiStatus);

    SpiStatus = Flash_W25Qx_WaitBusy(self,1,W25QX_CMD_TIMEOUT);
    RET_ERR(SpiStatus);

    SpiStatus = Flash_W25Qx_GetJedecId(self,&id);  
    RET_ERR(SpiStatus);
    
    if (id==0||id==(uint32_t)-1)
    {
        DBG_ERROR("Init:Flash_W25Qx_Reset Timeout\n");
        return FLASH_W25QX_ERR_TIMEOUT;
    }
    else if (id!=W25QX_JEDEC_ID)
    {
        DBG_WARNING("W25QX_JEDEC_ID not match\r\n");
        DBG_WARNING("Expect:%x ,Receive:%x\r\n",W25QX_JEDEC_ID,id);
    }
    return FLASH_W25QX_OK;
}

Flash_W25Qx_Status_t Flash_W25Qx_GetJedecId(Flash_W25Qx_t *self,uint32_t *id_out)
{    
    uint8_t id[4],cmd[4]={OP_READ_JEDEC_ID,0,0,0};
    Flash_W25Qx_Status_t SpiStatus;

    CS_LOW(self);
    SpiStatus = __w25qx_lowlevel_write_read(self,cmd,id,sizeof(cmd),W25QX_CMD_TIMEOUT);
    CS_HIGH(self);

    RET_ERR(SpiStatus);
    
    *id_out = (uint32_t)( (id[1]<<16) | (id[2]<<8) | (id[3]<<0));
    
    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_ReadStatusReg(Flash_W25Qx_t *self,bool cs_ctrl,uint8_t *reg_out)
{
    uint8_t cmd=OP_READ_STATUS_REG;
    Flash_W25Qx_Status_t SpiStatus;

    if(cs_ctrl) CS_LOW(self);
    SpiStatus = __w25qx_lowlevel_write(self,&cmd,sizeof(cmd),W25QX_CMD_TIMEOUT);
    SpiStatus = __w25qx_lowlevel_read(self,reg_out,sizeof(*reg_out),W25QX_CMD_TIMEOUT);
    if(cs_ctrl) CS_HIGH(self);
    
    RET_ERR(SpiStatus); 
    
    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_WaitBusy(Flash_W25Qx_t *self,bool cs_ctrl,uint16_t timeout)
{
    SysTimer_t tmr;
    uint8_t stat_reg=0;
    Flash_W25Qx_Status_t SpiStatus;

    if(cs_ctrl) CS_LOW(self);

    SysTimer_Init(&tmr,timeout);
    while(SysTimer_IsElapsed(&tmr)==false)
    {
        if(Flash_W25Qx_ReadStatusReg(self,false,&stat_reg)==FLASH_W25QX_OK)
        {
            if(stat_reg & 0x01)
                SpiStatus = FLASH_W25QX_ERR_BUSY;
            else
            {
                SpiStatus = FLASH_W25QX_OK;
                break;
            }
        }
    }

    if(cs_ctrl) CS_HIGH(self);        

    RET_ERR(SpiStatus);

    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_WriteEnable(Flash_W25Qx_t *self,bool en)
{
    uint8_t stat_reg,cmd=en? OP_WRITE_ENABLE:OP_WRITE_DISABLE;
    Flash_W25Qx_Status_t SpiStatus;

    CS_LOW(self);
    SpiStatus = __w25qx_lowlevel_write(self,&cmd,sizeof(cmd),W25QX_CMD_TIMEOUT);
    CS_HIGH(self);
    RET_ERR(SpiStatus);

    SpiStatus = Flash_W25Qx_ReadStatusReg(self,1,&stat_reg);
    RET_ERR(SpiStatus);

    stat_reg&=0x02; //WriteEn Bit
    SpiStatus = ((en>0)==(stat_reg>0)) ? FLASH_W25QX_OK : FLASH_W25QX_ERR;    
    RET_ERR(SpiStatus);

    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_Reset(Flash_W25Qx_t *self)
{
    uint8_t cmd;
    Flash_W25Qx_Status_t SpiStatus;

    cmd = OP_RESET_EN;
    CS_LOW(self);
    SpiStatus = __w25qx_lowlevel_write(self,&cmd,sizeof(cmd),W25QX_CMD_TIMEOUT);
    CS_HIGH(self);
    RET_ERR(SpiStatus);

    cmd = OP_RESET;
    CS_LOW(self);
    SpiStatus = __w25qx_lowlevel_write(self,&cmd,sizeof(cmd),W25QX_CMD_TIMEOUT);  
    CS_HIGH(self);

    delay(FLASH_W25QX_RESET_DELAY);
    
    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_EraseChip(Flash_W25Qx_t *self)
{
    uint8_t cmd=OP_ERASE_CHIP;
    Flash_W25Qx_Status_t SpiStatus;
    
    SpiStatus = Flash_W25Qx_WriteEnable(self,1);
    RET_ERR(SpiStatus);

    CS_LOW(self);
    SpiStatus = __w25qx_lowlevel_write(self,&cmd,sizeof(cmd),W25QX_CMD_TIMEOUT);      
    CS_HIGH(self);
    
    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_EraseSector_4K(Flash_W25Qx_t *self,uint32_t addr)
{
    Flash_W25Qx_Status_t SpiStatus;
    
#if W25QX_STORAGE_TOTAL_SIZE <= 16*1024*1024 //16MB addressable in 3 byte
    uint8_t cmd[4]={
        OP_ERASE_SECTOR,
        (addr>>16)&0xff,
        (addr>>8)&0xff,
        (addr>>0)&0xff
    };
#elif W25QX_STORAGE_TOTAL_SIZE <= 4096*1024*1024 //4GB addressable in 4 byte
    uint8_t cmd[5]= {
        OP_ERASE_SECTOR_ADDR4,
        (addr>>24)&0xff,
        (addr>>16)&0xff,
        (addr>>8)&0xff,
        (addr>>0)&0xff
    };
#endif

    addr = ADDR_TO_SECTOR_NUM(addr);

    SpiStatus = IS_LEN_VALID(addr,W25QX_SECTOR_SIZE)? FLASH_W25QX_OK : FLASH_W25QX_ERR_LEN;
    RET_ERR(SpiStatus);

    SpiStatus = Flash_W25Qx_WriteEnable(self,1);
    RET_ERR(SpiStatus);
    
    CS_LOW(self);
    SpiStatus = __w25qx_lowlevel_write(self,cmd,sizeof(cmd),W25QX_CMD_TIMEOUT);
    CS_HIGH(self);
    RET_ERR(SpiStatus);
    
    SpiStatus = Flash_W25Qx_WaitBusy(self,true,W25QX_SECTOR_ERASE_TIMEOUT);  
    RET_ERR(SpiStatus);

    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_EraseRange_4K(Flash_W25Qx_t *self,uint32_t addr,uint16_t len)
{
    Flash_W25Qx_Status_t SpiStatus;
    uint32_t addr_end;

    SpiStatus = IS_LEN_VALID(addr,len)? FLASH_W25QX_OK : FLASH_W25QX_ERR_LEN;
    RET_ERR(SpiStatus);

    addr_end = ADDR_TO_SECTOR_NUM(len + addr-1);
    addr = ADDR_TO_SECTOR_NUM(addr);

    for (; addr <= addr_end; addr+=W25QX_SECTOR_SIZE)
    {
        SpiStatus=Flash_W25Qx_EraseSector_4K(self,addr);
        RET_ERR(SpiStatus);
    }

    return SpiStatus;
}


Flash_W25Qx_Status_t Flash_W25Qx_Write(Flash_W25Qx_t *self,uint32_t addr,uint16_t len,uint8_t* data)
{
    uint16_t w_len;
    Flash_W25Qx_Status_t SpiStatus;

#if W25QX_STORAGE_TOTAL_SIZE <= 16*1024*1024 //16MB addressable in 3 byte
    uint8_t cmd[4]={OP_WRITE_DATA};
#elif W25QX_STORAGE_TOTAL_SIZE <= 4096*1024*1024 //4GB addressable in 4 byte
    uint8_t cmd[5]={OP_WRITE_DATA_ADDR4};
#endif

    SpiStatus = IS_LEN_VALID(addr,len)? FLASH_W25QX_OK : FLASH_W25QX_ERR_LEN;
    RET_ERR(SpiStatus);
    
    while(len)
    {
#if W25QX_STORAGE_TOTAL_SIZE <= 16*1024*1024 //16MB addressable in 3 byte
        cmd[1]=(addr>>16)&0xff;
        cmd[2]=(addr>> 8)&0xff;
        cmd[3]=(addr>> 0)&0xff;
#elif W25QX_STORAGE_TOTAL_SIZE <= 4096*1024*1024 //4GB addressable in 4 byte
        cmd[1]=(addr>>24)&0xff;
        cmd[2]=(addr>>16)&0xff;
        cmd[3]=(addr>> 8)&0xff;
        cmd[4]=(addr>> 0)&0xff;
#endif
        //w_len determine write length for per loop
        //if address is not page align, we limit write length till page end.
        //if address is page align, we limit write length to page size.
        w_len = BSP_MIN(len,IS_ADDR_PAGE_ALIGN(addr) ? 
        W25QX_PAGE_SIZE :  W25QX_PAGE_SIZE - (addr&(W25QX_PAGE_SIZE-1)));
                
        SpiStatus = Flash_W25Qx_WriteEnable(self,true);
        RET_ERR(SpiStatus);

        CS_LOW(self);
        SpiStatus = __w25qx_lowlevel_write(self,cmd,sizeof(cmd),W25QX_CMD_TIMEOUT);
        SpiStatus = __w25qx_lowlevel_write(self,data,w_len,W25QX_CMD_TIMEOUT);      
        CS_HIGH(self);
        RET_ERR(SpiStatus);

        data+=w_len;
        addr+=w_len;
        len-=w_len;

        SpiStatus = Flash_W25Qx_WaitBusy(self,true,W25QX_PAGEWRITE_TIMEOUT);
        RET_ERR(SpiStatus);
    }
    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_Write_AsEEPROM(Flash_W25Qx_t *self,uint32_t addr,uint16_t len,uint8_t* data)
{
    uint8_t buf[W25QX_SECTOR_SIZE];
    uint32_t Sector_HeadAddr,Sector_EdgeAddr,w_len;
    Flash_W25Qx_Status_t SpiStatus;


    while (len)
    {
        Sector_HeadAddr = ADDR_TO_SECTOR_NUM(addr);
        Sector_EdgeAddr = Sector_HeadAddr+W25QX_SECTOR_SIZE;
        
        //read back the data
        SpiStatus = Flash_W25Qx_Read(self,Sector_HeadAddr,sizeof(buf),buf);
        RET_ERR(SpiStatus);

        //if write data is over sector edge, we limit write length to sector edge.
        w_len = addr+len > Sector_EdgeAddr ? Sector_EdgeAddr - addr : len;        
        //overwrite the data in the buffer
        memcpy((void*)(buf+addr-Sector_HeadAddr),(void*)data,w_len);
        len-=w_len;
        data+=w_len;
        addr+=w_len;

        SpiStatus = Flash_W25Qx_EraseSector_4K(self,Sector_HeadAddr);
        RET_ERR(SpiStatus);

        SpiStatus = Flash_W25Qx_Write(self,Sector_HeadAddr,sizeof(buf),buf);
        RET_ERR(SpiStatus);
    }

    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_Read(Flash_W25Qx_t *self,uint32_t addr,uint16_t len,uint8_t* data)
{
    Flash_W25Qx_Status_t SpiStatus;
#if W25QX_STORAGE_TOTAL_SIZE <= 16*1024*1024 //16MB addressable in 3 byte
    #if FLASH_W25QX_FAST_READ
        uint8_t cmd[5] = {
            OP_FASTREAD_DATA,
            (addr>>16)&0xff,
            (addr>>8)&0xff,
            (addr>>0)&0xff,
            0x00//dummy byte
        };
    #else
        uint8_t cmd[4] = {
            OP_READ_DATA,
            (addr>>16)&0xff,
            (addr>>8)&0xff,
            (addr>>0)&0xff
        };
    #endif
#elif W25QX_STORAGE_TOTAL_SIZE <= 4096*1024*1024 //4GB addressable in 4 byte
    #if FLASH_W25QX_FAST_READ
    uint8_t cmd[6]={
        OP_FASTREAD_DATA_ADDR4,
        (addr>>24)&0xff,
        (addr>>16)&0xff,
        (addr>>8)&0xff,
        (addr>>0)&0xff,
        0x00//dummy byte
    };
    #else
    uint8_t cmd[5]={
        OP_READ_DATA_ADDR4,
        (addr>>24)&0xff,
        (addr>>16)&0xff,
        (addr>>8)&0xff,
        (addr>>0)&0xff
    };
    #endif
#endif

    CS_LOW(self);
    SpiStatus = __w25qx_lowlevel_write(self,cmd,sizeof(cmd),W25QX_CMD_TIMEOUT);    
    SpiStatus = __w25qx_lowlevel_read(self,data,len,W25QX_CMD_TIMEOUT);
    CS_HIGH(self);

    return SpiStatus;
}

//DEBUG
#if FLASH_W25QX_DEBUG
Flash_W25Qx_Status_t Flash_W25Qx_DumpSector(Flash_W25Qx_t *self,uint32_t addr)
{
    uint8_t i,DumpBuf[DumpBuf_Size];
    uint16_t len = W25QX_SECTOR_SIZE,r_len;
    
    Flash_W25Qx_Status_t SpiStatus;

    addr=ADDR_TO_SECTOR_NUM(addr);
    DBG_INFO("----------DumpSector:%d, addr:0x%x\r\n",addr/W25QX_SECTOR_SIZE,addr);	
	DBG_INFO("          ___0___1___2___3___4___5___6___7___8___9___A___B___C___D___E___F\r\n");

    while (len)
    {
        r_len = len>DumpBuf_Size?DumpBuf_Size:len;        
        SpiStatus = Flash_W25Qx_Read(self,addr,r_len,DumpBuf);
        RET_ERR(SpiStatus);
        DBG_INFO("%08x| ",addr);

        for (i = 0; i < DumpBuf_Size; i++)
            DBG_INFO("%02x ",DumpBuf[i]);

        DBG_INFO("\n");
        len-=r_len;
        addr+=r_len;
    }

    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25Qx_Test(Flash_W25Qx_t *self)
{
    uint32_t Len,Addr;
    uint8_t W_Data[FLASH_W25QX_TESTLEN_MAX]={0};
    uint8_t R_Data[FLASH_W25QX_TESTLEN_MAX]={0};
    Flash_W25Qx_Status_t SpiStatus;

    Len = FLASH_W25QX_RANDOM(FLASH_W25QX_TESTLEN_MIN,FLASH_W25QX_TESTLEN_MAX);
    Addr = FLASH_W25QX_RANDOM(0,W25QX_STORAGE_TOTAL_SIZE - Len);  
    
    SpiStatus = Flash_W25QX_Test_RW(self,Len,ADDR_TO_SECTOR_NUM(Addr),W_Data,R_Data);
    RET_ERR(SpiStatus);    
    
    SpiStatus = Flash_W25QX_Test_RW_EEPROM(self,Len,Addr,W_Data,R_Data);
    RET_ERR(SpiStatus);

    return SpiStatus;
}

Flash_W25Qx_Status_t Flash_W25QX_Test_RW(Flash_W25Qx_t *self,uint32_t Len,uint32_t Addr,uint8_t *W_Data,uint8_t *R_Data)
{
    uint32_t i;
    Flash_W25Qx_Status_t SpiStatus=FLASH_W25QX_OK;
    DBG_INFO("W25QX_Test_RW,Addr:0x%x,Len:%d...",Addr,Len);

    SpiStatus = Flash_W25Qx_EraseRange_4K(self,Addr,Len);
    RET_ERR(SpiStatus);

    //fill random w_data
    for (i = 0; i < Len; i++)
        W_Data[i]=(uint8_t)rand();

    //write to flash
    SpiStatus=Flash_W25Qx_Write(self,Addr,Len,W_Data);
    RET_ERR(SpiStatus);

    //read from flash
    SpiStatus=Flash_W25Qx_Read(self,Addr,Len,R_Data);
    RET_ERR(SpiStatus);

    if(BSP_MEMCMP(W_Data,R_Data,Len)==0)
    {
        DBG_INFO("Success\n");
        return FLASH_W25QX_OK;
    }
    else
    {
        DBG_INFO("Fail\n");
        return FLASH_W25QX_ERR;
    }
}

Flash_W25Qx_Status_t Flash_W25QX_Test_RW_EEPROM(Flash_W25Qx_t *self,uint32_t Len,uint32_t Addr,uint8_t *W_Data,uint8_t *R_Data)
{
    uint32_t i;
    Flash_W25Qx_Status_t SpiStatus;
    DBG_INFO("W25QX_Test_RW_EEPROM,at:0x%x,Len:%d.....",Addr,Len);
    
    //random data
    for (i = 0; i < Len; i++)
        W_Data[i]=  (uint8_t)rand();

    //write as eeprom
    SpiStatus = Flash_W25Qx_Write_AsEEPROM(self,Addr,Len,W_Data);
    RET_ERR(SpiStatus);

    //read from flash
    SpiStatus=Flash_W25Qx_Read(self,Addr,Len,R_Data);
    RET_ERR(SpiStatus);

    if(BSP_MEMCMP(W_Data,R_Data,Len)==0)
    {
        DBG_INFO("Success\n");
        return FLASH_W25QX_OK;
    }
    else
    {
        DBG_INFO("Fail\n");
        return FLASH_W25QX_ERR;
    }
}
#endif
