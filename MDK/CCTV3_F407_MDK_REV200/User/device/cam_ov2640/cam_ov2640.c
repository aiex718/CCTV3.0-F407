#include "device/cam_ov2640/cam_ov2640.h"
#include "device/cam_ov2640/cam_ov2640_regs.h"

#include "bsp/sys/dbg_serial.h"
#include "bsp/hal/i2c.h"
#include "bsp/hal/dcmi.h"
#include "bsp/sys/systime.h"

//private helper macro
#define __WriteRegArray(self,RegArray) do{\
    for (i = 0; i < (sizeof(RegArray) / 2); i++) \
        Device_CamOV2640_WriteReg(self,RegArray[i][0], RegArray[i][1]); \
}while(0)

//private function
static bool Device_CamOV2640_WriteReg(Device_CamOV2640_t* self,uint8_t addr, uint8_t data)
{
    uint8_t buffer[2] = {addr, data};
    return HAL_I2C_Write_Polling(self->CamOV2640_I2C,CAM_OV2640_WRITE_ADDRESS,
        buffer,sizeof(buffer),CAM_OV2640_TIMEOUT) == HAL_I2C_OK;
}
static uint8_t Device_CamOV2640_ReadReg(Device_CamOV2640_t* self,uint8_t addr)
{
    uint8_t buffer;
    HAL_I2C_Write_Polling(self->CamOV2640_I2C,CAM_OV2640_WRITE_ADDRESS,&addr,sizeof(addr),CAM_OV2640_TIMEOUT);  
    HAL_I2C_Read_Polling(self->CamOV2640_I2C,CAM_OV2640_READ_ADDRESS,&buffer,sizeof(buffer),CAM_OV2640_TIMEOUT);
    return buffer;
}

//callbacks
void Device_CamOV2640_DcmiFrameCallback(void *sender,void *arg,void *owner)
{
    BSP_UNUSED_ARG(sender);
    BSP_UNUSED_ARG(arg);
    Device_CamOV2640_t* self = (Device_CamOV2640_t*)owner;
    HAL_DMA_Cmd(self->CamOV2640_DCMI->DCMI_RxDma_Cfg,DISABLE);
}

void Device_CamOV2640_DcmiRxDmaTcCallback(void *sender,void *arg,void *owner)
{
    static const uint8_t Jpeg_Start_Tag[]={0xFF,0xD8};
    static const uint8_t Jpeg_End_Tag[]={0xFF,0xD9};
    HAL_DMA_t *dma = (HAL_DMA_t*)sender;
    Device_CamOV2640_t* self = (Device_CamOV2640_t*)owner;
    uint8_t *rxbuf = self->CamOV2640_Buffer;
    uint16_t rxlen = self->CamOV2640_Buffer_Len - HAL_DMA_GetNumOfData(dma)*4;
    uint16_t frame_len=0;

    //check jpeg start and end tag
    if(rxlen && BSP_MEMCMP(rxbuf,Jpeg_Start_Tag,BSP_ARR_LEN(Jpeg_Start_Tag))==0)
    {
        uint8_t *tail = BSP_MEMSEARCH(rxbuf,rxlen,Jpeg_End_Tag,BSP_ARR_LEN(Jpeg_End_Tag),true);
        if(tail!=NULL)
        {
            tail+=BSP_ARR_LEN(Jpeg_End_Tag);
            if(tail>rxbuf)
                frame_len = tail-rxbuf;
        }
    }

    //Check if frame is valid, there're 2 case bad frame is expected.
    //1. It's first frame we received
    //2. Cam is disabled(during capturing)
    //error message is omitted when bad frame is expected.

    #if DEVICE_CAM_OV2640_DEBUG
    DBG_INFO("%6d:Snaped len:%u ,trimmed len:%u\n",
        SysTime_Get(),rxlen,frame_len);
    #endif

    if (frame_len==0 && Device_CamOV2640_IsEnabled(self))
    {
        //receive bad frame, retry capture using same buffer
        Device_CamOV2640_SetBuf(self,rxbuf,self->CamOV2640_Buffer_Len);
        Device_CamOV2640_CaptureCmd(self,true);

        //only show warning when it's not first frame
        if(self->_is_first_frame==false)
        {
            DBG_WARNING("Bad frame,Snaped len:%u ,trimmed len:%u, retrying.\n",
                rxlen,frame_len);
        }
    }
    else// equals to if(frame_len || Device_CamOV2640_IsEnabled(self)==false)
    {
        // We invoke callback when frame is valid, or cam is disabled
        // (to return frame buffer)
        self->CamOV2640_Buffer_Len = frame_len;
        Callback_InvokeNowOrPending_Idx(self ,NULL, self->CamOV2640_Callbacks,
            CAMOV2640_CALLBACK_NEWFRAME,self->_callback_pending_flag);
    }

    self->_is_first_frame = !Device_CamOV2640_IsEnabled(self);

    BSP_UNUSED_ARG(arg);
    //uncomment this if self looping mode is needed
    //Device_CamOV2640_CaptureCmd(self,true);
}

//common function
void Device_CamOV2640_Init(Device_CamOV2640_t* self)
{
    CAM_OV2640_ID_t id;
    
    if(self->CamOV2640_PWDN_pin!=NULL)
    {
        HAL_GPIO_InitPin(self->CamOV2640_PWDN_pin);
        HAL_GPIO_WritePin(self->CamOV2640_PWDN_pin,0);
    }

    HAL_DCMI_Init(self->CamOV2640_DCMI);
    HAL_DCMI_JpegCmd(self->CamOV2640_DCMI,true);
    HAL_I2C_Init(self->CamOV2640_I2C);   
    HAL_I2C_Cmd(self->CamOV2640_I2C, true);

    Device_CamOV2640_ReadID(self,&id);
    DBG_INFO("OV2640 ID: %x %x %x %x\n", id.Manufacturer_ID1, id.Manufacturer_ID2, id.PIDH, id.PIDL);
    Device_CamOV2640_SetJpegFormat(self,JPEG_320x240);

    self->_is_first_frame = true;

    BSP_ARR_CLEAR(self->CamOV2640_Callbacks);
    self->pExtension = NULL;
    self->_callback_pending_flag = 0;

    //regist callbacks
    self->_dcmi_frame_cb.func = Device_CamOV2640_DcmiFrameCallback;
    self->_dcmi_frame_cb.owner = self;
    self->_dcmi_frame_cb.invoke_cfg = INVOKE_IMMEDIATELY;
    HAL_DCMI_SetCallback(self->CamOV2640_DCMI,DCMI_CALLBACK_FRAME,&self->_dcmi_frame_cb);

    self->_dcmi_rxdma_tc_cb.func = Device_CamOV2640_DcmiRxDmaTcCallback;
    self->_dcmi_rxdma_tc_cb.owner = self;
    self->_dcmi_rxdma_tc_cb.invoke_cfg = INVOKE_IMMEDIATELY;
    HAL_DMA_SetCallback(self->CamOV2640_DCMI->DCMI_RxDma_Cfg,DMA_CALLBACK_IRQ_TC,&self->_dcmi_rxdma_tc_cb);
}

void Device_CamOV2640_Attach_DCMI(Device_CamOV2640_t* self,HAL_DCMI_t* dcmi)
{
    self->CamOV2640_DCMI = dcmi;
}

void Device_CamOV2640_Attach_I2C(Device_CamOV2640_t* self,HAL_I2C_t* i2c)
{
    self->CamOV2640_I2C = i2c;
}

void Device_CamOV2640_SetCallback(Device_CamOV2640_t* self, Device_CamOV2640_CallbackIdx_t cb_idx, Callback_t* callback)
{
    if(cb_idx < __NOT_CALLBACK_CAM_OV2640_MAX)
        self->CamOV2640_Callbacks[cb_idx] = callback;
}

void Device_CamOV2640_Service(Device_CamOV2640_t* self)
{
    while(self->_callback_pending_flag)
    {
        uint8_t cb_idx = BitFlag_BinToIdx(self->_callback_pending_flag);
        Callback_Invoke_Idx(self,NULL,self->CamOV2640_Callbacks,cb_idx);        
        BitFlag_ClearIdx(self->_callback_pending_flag,cb_idx);
    }
}

//device specific function
Device_CamOV2640_Status_t Device_CamOV2640_CaptureCmd(Device_CamOV2640_t* self,bool en)
{
    if(en)
    {
        if(self->CamOV2640_Buffer==NULL||self->CamOV2640_Buffer_Len==0)
            return DEVICE_CAMOV2640_ERR_ARG;
        if(HAL_DCMI_IsEnabled(self->CamOV2640_DCMI)==false)
            return DEVICE_CAMOV2640_ERR_DISABLED;
        if(HAL_DCMI_IsCapturing(self->CamOV2640_DCMI))
            return DEVICE_CAMOV2640_ERR_BUSY;
        
        HAL_DCMI_StartDmaRecv(self->CamOV2640_DCMI,
            self->CamOV2640_Buffer,self->CamOV2640_Buffer_Len);
        HAL_DCMI_CaptureCmd(self->CamOV2640_DCMI,true);
    }
    else
    {
        HAL_DCMI_CaptureCmd(self->CamOV2640_DCMI,false);
    }

    return DEVICE_CAMOV2640_OK;
}

void Device_CamOV2640_SetBuf(Device_CamOV2640_t* self,uint8_t *buf, uint16_t len)
{
    if(HAL_DCMI_IsCapturing(self->CamOV2640_DCMI)==false)
    {
        self->CamOV2640_Buffer = buf;
        self->CamOV2640_Buffer_Len = len;
    }
}

void Device_CamOV2640_Cmd(Device_CamOV2640_t* self,bool en)
{
    HAL_DCMI_Cmd(self->CamOV2640_DCMI,en);

    if(self->CamOV2640_PWDN_pin!=NULL)
        HAL_GPIO_WritePin(self->CamOV2640_PWDN_pin,!en);
}

bool Device_CamOV2640_IsEnabled(Device_CamOV2640_t* self)
{
    return HAL_DCMI_IsEnabled(self->CamOV2640_DCMI);
}

bool Device_CamOV2640_IsCapturing(Device_CamOV2640_t* self)
{
    return HAL_DCMI_IsCapturing(self->CamOV2640_DCMI);
}

void Device_CamOV2640_SoftReset(Device_CamOV2640_t* self)
{
    Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x01);
    Device_CamOV2640_WriteReg(self,OV2640_SENSOR_COM7, 0x80);
}

void Device_CamOV2640_ReadID(Device_CamOV2640_t* self,CAM_OV2640_ID_t *id)
{
    Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x01);
    id->Manufacturer_ID1 = Device_CamOV2640_ReadReg(self,OV2640_SENSOR_MIDH);
    id->Manufacturer_ID2 = Device_CamOV2640_ReadReg(self,OV2640_SENSOR_MIDL);
    id->PIDH = Device_CamOV2640_ReadReg(self,OV2640_SENSOR_PIDH);
    id->PIDL = Device_CamOV2640_ReadReg(self,OV2640_SENSOR_PIDL);
}

void Device_CamOV2640_SetJpegFormat(Device_CamOV2640_t* self,CAM_OV2640_JpegFormat_Config_t jpeg_format)
{
    uint32_t i;

    Device_CamOV2640_SoftReset(self);
    delay(200);

    /* Initialize OV2640 */
    __WriteRegArray(self,CAM_OV2640_JPEG_INIT);

    /* Set to output YUV422 */
    __WriteRegArray(self,CAM_OV2640_YUV422);

    Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x01);
    Device_CamOV2640_WriteReg(self,OV2640_SENSOR_COM10, 0x00);

    /* Set to output JPEG */
    __WriteRegArray(self,CAM_OV2640_JPEG);

    delay(100);

    switch (jpeg_format)
    {
        case JPEG_160x120:
            __WriteRegArray(self,CAM_OV2640_160x120_JPEG);
            break;
        case JPEG_176x144:
            __WriteRegArray(self,CAM_OV2640_176x144_JPEG);
            break;
        case JPEG_320x240:
            __WriteRegArray(self,CAM_OV2640_320x240_JPEG);
            break;
        case JPEG_352x288:
            __WriteRegArray(self,CAM_OV2640_352x288_JPEG);
            break;
        default:
            break;
    }
}

void Device_CamOV2640_SetQs(Device_CamOV2640_t* self,uint8_t qs)
{
    //TODO:Set QS, default 0x0C(12), smaller is better
    Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_Qs, qs);
}

void Device_CamOV2640_SetClock(Device_CamOV2640_t* self,bool doubler,uint8_t div)
{
    uint8_t clkrc;

    Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x01);
    clkrc = Device_CamOV2640_ReadReg(self,OV2640_SENSOR_CLKRC);
    
    if(doubler)
        clkrc |= 0x80;
    else
        clkrc &= ~0x80;

    clkrc |= div & 0x1F;//Mask 5 bit

    Device_CamOV2640_WriteReg(self,OV2640_SENSOR_CLKRC,clkrc);
}

void Device_CamOV2640_SetFlip(Device_CamOV2640_t* self,bool vert,bool hori)
{
    uint8_t reg04;
    Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x01);
    reg04 = Device_CamOV2640_ReadReg(self,OV2640_SENSOR_REG04);

    if(hori)
        reg04 |= 0x80;
    else
        reg04 &= ~0x80;
    
    if(vert)
        reg04 |= 0x40;
    else
        reg04 &= ~0x40;
    
    Device_CamOV2640_WriteReg(self,OV2640_SENSOR_REG04,reg04);
}

void Device_CamOV2640_SetBrightness(Device_CamOV2640_t* self,CAM_OV2640_Brightness_Config_t brightness)
{
    Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPADDR, 0x00);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPDATA, 0x04);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPADDR, 0x09);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPDATA, (uint8_t)brightness);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPDATA, 0x00);
}

void Device_CamOV2640_SetContrast(Device_CamOV2640_t* self,CAM_OV2640_Contrast_Config_t contrast)
{
    uint8_t value1, value2;
    
    switch(contrast)
    {
        case CONTRAST_P2:
        value1 = 0x28;
        value2 = 0x0C;
        break;
        
        case CONTRAST_P1:
        value1 = 0x24;
        value2 = 0x16;
        break;
        
        case CONTRAST_NORMAL:
        value1 = 0x20;
        value2 = 0x20;
        break;

        case CONTRAST_N1:
        value1 = 0x1c;
        value2 = 0x2a;
        break;

        case CONTRAST_N2:
        value1 = 0x18;
        value2 = 0x34;
        break;
    }

    Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPADDR, 0x00);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPDATA, 0x04);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPADDR, 0x07);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPDATA, 0x20);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPDATA, value1);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPDATA, value2);
    Device_CamOV2640_WriteReg(self,OV2640_DSP_BPDATA, 0x06);
}

void Device_CamOV2640_SetLightMode(Device_CamOV2640_t* self,CAM_OV2640_LightMode_Config_t light)
{
    switch(light)
    {
        case LIGHTMODE_AUTO:     //Auto
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0xc7, 0x00); //AWB on
        break;
        
        case LIGHTMODE_SUNNY:     //Sunny
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0xc7, 0x40); //AWB off
        Device_CamOV2640_WriteReg(self,0xcc, 0x5e);
        Device_CamOV2640_WriteReg(self,0xcd, 0x41);
        Device_CamOV2640_WriteReg(self,0xce, 0x54);
        
        break;
        
        case LIGHTMODE_CLOUDY:    //Cloudy 
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0xc7, 0x40); //AWB off
        Device_CamOV2640_WriteReg(self,0xcc, 0x65);
        Device_CamOV2640_WriteReg(self,0xcd, 0x41);
        Device_CamOV2640_WriteReg(self,0xce, 0x4f);
        break;
        
        case LIGHTMODE_OFFICE:   //Office
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0xc7, 0x40); //AWB off
        Device_CamOV2640_WriteReg(self,0xcc, 0x52);
        Device_CamOV2640_WriteReg(self,0xcd, 0x41);
        Device_CamOV2640_WriteReg(self,0xce, 0x66);
        break;
        
        case LIGHTMODE_HOME:   //Home
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0xc7, 0x40); //AWB off
        Device_CamOV2640_WriteReg(self,0xcc, 0x42);
        Device_CamOV2640_WriteReg(self,0xcd, 0x3f);
        Device_CamOV2640_WriteReg(self,0xce, 0x71);
        break;

    }
}

void Device_CamOV2640_SetSpecialEffects(Device_CamOV2640_t* self,CAM_OV2640_SpecialEffects_Config_t effect)
{
    switch(effect)
    {
        case SPECIALEFFECT_ANTIQUE:// Antique
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x00);
        Device_CamOV2640_WriteReg(self,0x7d, 0x18);
        Device_CamOV2640_WriteReg(self,0x7c, 0x05);
        Device_CamOV2640_WriteReg(self,0x7d, 0x40);
        Device_CamOV2640_WriteReg(self,0x7d, 0xa6);
        break;
        
        case SPECIALEFFECT_BLUISH: //Bluish
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x00);
        Device_CamOV2640_WriteReg(self,0x7d, 0x18);
        Device_CamOV2640_WriteReg(self,0x7c,0x05);
        Device_CamOV2640_WriteReg(self,0x7d, 0xa0);
        Device_CamOV2640_WriteReg(self,0x7d, 0x40);
        
        break;
        
        case SPECIALEFFECT_GREENISH://Greenish
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x00);
        Device_CamOV2640_WriteReg(self,0x7d, 0x18);
        Device_CamOV2640_WriteReg(self,0x7c, 0x05);
        Device_CamOV2640_WriteReg(self,0x7d, 0x40);
        Device_CamOV2640_WriteReg(self,0x7d, 0x40);
        break;
        
        case SPECIALEFFECT_REDDISH:// Reddish
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x00);
        Device_CamOV2640_WriteReg(self,0x7d, 0x18);
        Device_CamOV2640_WriteReg(self,0x7c, 0x05);
        Device_CamOV2640_WriteReg(self,0x7d, 0x40);
        Device_CamOV2640_WriteReg(self,0x7d, 0xc0);
        break;
        
        case SPECIALEFFECT_BLACKWHITE:// B&W
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x00);
        Device_CamOV2640_WriteReg(self,0x7d, 0x18);
        Device_CamOV2640_WriteReg(self,0x7c, 0x05);
        Device_CamOV2640_WriteReg(self,0x7d, 0x80);
        Device_CamOV2640_WriteReg(self,0x7d, 0x80);
        break;
        
        case SPECIALEFFECT_NEGATIVE://Negative
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x00);
        Device_CamOV2640_WriteReg(self,0x7d, 0x40);
        Device_CamOV2640_WriteReg(self,0x7c, 0x05);
        Device_CamOV2640_WriteReg(self,0x7d, 0x80);
        Device_CamOV2640_WriteReg(self,0x7d, 0x80);
        break;
        
        case SPECIALEFFECT_BLACKWHITE_NEGATIVE://B&W negative
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x00);
        Device_CamOV2640_WriteReg(self,0x7d, 0x58);
        Device_CamOV2640_WriteReg(self,0x7c, 0x05);
        Device_CamOV2640_WriteReg(self,0x7d, 0x80);
        Device_CamOV2640_WriteReg(self,0x7d, 0x80);
        break;
        
        case SPECIALEFFECT_NORMAL://Normal
        Device_CamOV2640_WriteReg(self,OV2640_DSP_RA_DLMT, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x00);
        Device_CamOV2640_WriteReg(self,0x7d, 0x00);
        Device_CamOV2640_WriteReg(self,0x7c, 0x05);
        Device_CamOV2640_WriteReg(self,0x7d, 0x80);
        Device_CamOV2640_WriteReg(self,0x7d, 0x80);
        break;

        default:
        break;
    }
}
