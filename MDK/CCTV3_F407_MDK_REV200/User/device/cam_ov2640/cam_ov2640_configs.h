#ifndef CAM_OV2640_CONFIGS_H
#define CAM_OV2640_CONFIGS_H

#include "bsp/platform/platform_defs.h"

#ifndef CAM_OV2640_WRITE_ADDRESS
    #define CAM_OV2640_WRITE_ADDRESS    0x60
#endif

#ifndef CAM_OV2640_READ_ADDRESS
    #define CAM_OV2640_READ_ADDRESS     0x61
#endif 

#ifndef CAM_OV2640_TIMEOUT
    #define CAM_OV2640_TIMEOUT     1000
#endif 

typedef struct CAM_OV2640_ID_s
{
  uint8_t Manufacturer_ID1;
  uint8_t Manufacturer_ID2;
  uint8_t PIDH;
  uint8_t PIDL;
}CAM_OV2640_ID_t;

typedef enum   
{
    JPEG_160x120  = 0 ,
    JPEG_176x144      ,
    JPEG_320x240      ,
    JPEG_352x288      ,
    __JPEGFORMAT_CONFIG_MAX   
}CAM_OV2640_JpegFormat_Config_t;

typedef enum   
{
    BRIGHTNESS_P2       = 0 ,
    BRIGHTNESS_P1           ,
    BRIGHTNESS_NORMAL       ,
    BRIGHTNESS_N1           ,
    BRIGHTNESS_N2           ,
    __BRIGHTNESS_CONFIG_MAX
}CAM_OV2640_Brightness_Config_t;


typedef enum   
{
    CONTRAST_P2         = 0 ,
    CONTRAST_P1             ,
    CONTRAST_NORMAL         ,
    CONTRAST_N1             ,
    CONTRAST_N2             ,
    __CONTRAST_CONFIG_MAX
}CAM_OV2640_Contrast_Config_t;

typedef enum   
{
    LIGHTMODE_AUTO      = 0 ,
    LIGHTMODE_SUNNY         ,
    LIGHTMODE_CLOUDY        ,
    LIGHTMODE_OFFICE        ,
    LIGHTMODE_HOME          ,
    __LIGHTMODE_CONFIG_MAX
}CAM_OV2640_LightMode_Config_t;

typedef enum   
{
    SPECIALEFFECT_ANTIQUE               = 0 ,
    SPECIALEFFECT_BLUISH                    ,
    SPECIALEFFECT_GREENISH                  ,
    SPECIALEFFECT_REDDISH                   ,
    SPECIALEFFECT_BLACKWHITE                ,
    SPECIALEFFECT_NEGATIVE                  ,
    SPECIALEFFECT_BLACKWHITE_NEGATIVE       ,
    SPECIALEFFECT_NORMAL                    ,
    __SPECIALEFFECTS_CONFIG_MAX
}CAM_OV2640_SpecialEffects_Config_t;

//init configs 
extern const uint8_t CAM_OV2640_JPEG_INIT[190][2];
extern const uint8_t CAM_OV2640_YUV422[9][2];
extern const uint8_t CAM_OV2640_JPEG[8][2];
extern const uint8_t CAM_OV2640_160x120_JPEG[39][2];
extern const uint8_t CAM_OV2640_176x144_JPEG[39][2];
extern const uint8_t CAM_OV2640_320x240_JPEG[39][2];
extern const uint8_t CAM_OV2640_352x288_JPEG[39][2];

#endif
