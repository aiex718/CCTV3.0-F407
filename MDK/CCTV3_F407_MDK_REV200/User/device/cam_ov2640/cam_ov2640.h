#ifndef CAM_OV2640_H
#define CAM_OV2640_H

#include "bsp/platform/platform_defs.h"

#include "bsp/hal/gpio.h"
#include "bsp/hal/i2c.h"
#include "bsp/hal/dcmi.h"
#include "bsp/sys/bitflag.h"
#include "bsp/sys/callback.h"

#include "device/cam_ov2640/cam_ov2640_configs.h"

#ifndef DEVICE_CAM_OV2640_DEBUG
    #define DEVICE_CAM_OV2640_DEBUG 0
#endif


typedef enum 
{
    DEVICE_CAMOV2640_OK = 0,
    DEVICE_CAMOV2640_ERR_DISABLED,
    DEVICE_CAMOV2640_ERR_ARG,
    DEVICE_CAMOV2640_ERR_BUSY,
    DEVICE_CAMOV2640_ERR,
}Device_CamOV2640_Status_t;

typedef enum 
{
    //source=Device_CamOV2640_t,arg=NULL
    CAMOV2640_CALLBACK_NEWFRAME = 0 ,
    __NOT_CALLBACK_CAM_OV2640_MAX   ,
}Device_CamOV2640_CallbackIdx_t;

typedef struct Device_CamOV2640_s
{
    HAL_DCMI_t *CamOV2640_DCMI;
    HAL_I2C_t *CamOV2640_I2C;
    HAL_GPIO_pin_t *CamOV2640_PWDN_pin;
    //frame buffer
    uint8_t *CamOV2640_Buffer;
    uint16_t CamOV2640_Buffer_Len;
    //callbalck
    Callback_t *CamOV2640_Callbacks[__NOT_CALLBACK_CAM_OV2640_MAX];
    //custom data
    void *pExtension;

    //flag for prevent first frame error
    __IO bool _is_first_frame;
    //private flags, dont use
    BitFlag8_t _callback_pending_flag;
    //callbacks, dont modify
    Callback_t _dcmi_frame_cb, _dcmi_rxdma_tc_cb;
} Device_CamOV2640_t;

//macro
#define Device_CamOV2640_IsSnaping(self) \
    (self->CamOV2640_DCMI->DCMIx->CR & DCMI_CR_CAPTURE != 0)

//common function
void Device_CamOV2640_Init(Device_CamOV2640_t* self);
void Device_CamOV2640_Attach_DCMI(Device_CamOV2640_t* self,HAL_DCMI_t* dcmi);
void Device_CamOV2640_Attach_I2C(Device_CamOV2640_t* self,HAL_I2C_t* i2c);
void Device_CamOV2640_SetCallback(Device_CamOV2640_t* self, \
    Device_CamOV2640_CallbackIdx_t cb_idx, Callback_t* callback); 
void Device_CamOV2640_Service(Device_CamOV2640_t* self);


//device specific function
Device_CamOV2640_Status_t Device_CamOV2640_CaptureCmd(Device_CamOV2640_t* self,bool en);
void Device_CamOV2640_SetBuf(Device_CamOV2640_t* self,uint8_t *buf, uint16_t len);
void Device_CamOV2640_Cmd(Device_CamOV2640_t* self,bool en);
bool Device_CamOV2640_IsEnabled(Device_CamOV2640_t* self);
bool Device_CamOV2640_IsCapturing(Device_CamOV2640_t* self);
void Device_CamOV2640_SoftReset(Device_CamOV2640_t* self);
void Device_CamOV2640_ReadID(Device_CamOV2640_t* self,CAM_OV2640_ID_t *id);
void Device_CamOV2640_SetJpegFormat(Device_CamOV2640_t* self,CAM_OV2640_JpegFormat_Config_t jpeg_format);
void Device_CamOV2640_SetQs(Device_CamOV2640_t* self,uint8_t qs);
void Device_CamOV2640_SetClock(Device_CamOV2640_t* self,bool doubler,uint8_t div);
void Device_CamOV2640_SetFlip(Device_CamOV2640_t* self,bool vert,bool hori);
void Device_CamOV2640_SetBrightness(Device_CamOV2640_t* self,CAM_OV2640_Brightness_Config_t brightness);
void Device_CamOV2640_SetContrast(Device_CamOV2640_t* self,CAM_OV2640_Contrast_Config_t contrast);
void Device_CamOV2640_SetLightMode(Device_CamOV2640_t* self,CAM_OV2640_LightMode_Config_t light);
void Device_CamOV2640_SetSpecialEffects(Device_CamOV2640_t* self,CAM_OV2640_SpecialEffects_Config_t effect);




#endif
