#ifndef CURRENT_TRIG_H
#define CURRENT_TRIG_H

#include "bsp/platform/platform_defs.h"

#include "bsp/hal/dma.h"
#include "bsp/hal/adc.h"
#include "bsp/hal/timer.h"
#include "bsp/sys/bitflag.h"

//debug
#ifndef CURRENT_TRIG_DEBUG
    #define CURRENT_TRIG_DEBUG 0
#endif

#ifndef CURRENT_TRIG_DEBUG_PRINT_EVENT
    #define CURRENT_TRIG_DEBUG_PRINT_EVENT 0
#endif

#ifndef CURRENT_TRIG_DEBUG_PRINT_SIGNAL
    #define CURRENT_TRIG_DEBUG_PRINT_SIGNAL 0
#endif

#ifndef CURRENT_TRIG_DEBUG_PRINT_ADC_VALUE
    #define CURRENT_TRIG_DEBUG_PRINT_ADC_VALUE 0
#endif

// Peak detection scan params
#ifndef CURRENT_TRIG_SCAN_ROLLBACK
    #define CURRENT_TRIG_SCAN_ROLLBACK 0
#endif

#ifndef CURRENT_TRIG_SCAN_WINDOW
    #error "CURRENT_TRIG_SCAN_WINDOW not defined"
#endif

//ADC
#ifndef CURRENT_TRIG_ADC_VAL_TYPE
    #define CURRENT_TRIG_ADC_VAL_TYPE uint8_t
#endif

#ifndef CURRENT_TRIG_ADC_RESOLUTION
    #define CURRENT_TRIG_ADC_RESOLUTION (1<<8)
#endif

#ifndef CURRENT_TRIG_ADC_VREF
    #define CURRENT_TRIG_ADC_VREF 3.3F
#endif

//Sensing resistor, normally 100 ohm
#ifndef CURRENT_TRIG_SENSING_RESISTOR_VAL
    #error "CURRENT_TRIG_SENSING_RESISTOR_VAL not defined"
#endif

#ifndef CURRENT_TRIG_MAX_CURRENT_MA
    #define CURRENT_TRIG_MAX_CURRENT_MA 20 //20ma
#endif

#ifndef CURRENT_TRIG_FLOAT_TYPE
    #define CURRENT_TRIG_FLOAT_TYPE float
#endif

#ifndef CURRENT_TRIG_DIFF_MIN
    #define CURRENT_TRIG_DIFF_MIN 0.1F
#endif


typedef struct Device_CurrentTrig_ConfigFile_s
{
    uint8_t CurrentTrig_Disconnect_Thres_mA;//0~20
    uint8_t CurrentTrig_Overload_Thres_mA;//0~20
    uint8_t __padding[2];
    uint16_t CurrentTrig_PeakThreshold_1000x;//this value is 1000x of the real value
    uint16_t CurrentTrig_PeakInfluence_1000x;//this value is 1000x of the real value
}Device_CurrentTrig_ConfigFile_t;

typedef enum 
{
    //ADC callbacks always invoked in service call
    DEVICE_CURRENT_TRIG_CALLBACK_DISCONNECT     = 0 ,
    DEVICE_CURRENT_TRIG_CALLBACK_TRIGGERED          ,
    DEVICE_CURRENT_TRIG_CALLBACK_OVERLOAD           ,
    __NOT_CALLBACK_CURRENTTRIG_MAX                  ,
}Device_CurrentTrig_CallbackIdx_t;

typedef struct Device_CurrentTrig_s
{
    //Timer TRGO
    HAL_Timer_t *CurrentTrig_Timer;
    //ADC
    HAL_ADC_t *CurrentTrig_ADC;
    //DMA
    HAL_DMA_t *CurrentTrig_DMA;
    //Callbacks 
    Callback_t *CurrentTrig_Callbacks[__NOT_CALLBACK_CURRENTTRIG_MAX];

    //Thresholds
    //Current lower than this will trigger DISCONNECT callback
    uint8_t CurrentTrig_Disconnect_Thres_mA;
    //ADC raw value higher than this will trigger OVERLOAD callback
    uint8_t CurrentTrig_Overload_Thres_mA;

    //Peak detection params
    CURRENT_TRIG_FLOAT_TYPE CurrentTrig_PeakThreshold;
    CURRENT_TRIG_FLOAT_TYPE CurrentTrig_PeakInfluence;

    //Peak detection data buffer
    CURRENT_TRIG_FLOAT_TYPE *CurrentTrig_Val_Buf;
    uint16_t CurrentTrig_Val_Buf_Len;

    //ADC raw scan value buffer for dma recv
    CURRENT_TRIG_ADC_VAL_TYPE *CurrentTrig_ADC_Buf;
    uint16_t CurrentTrig_ADC_Buf_Len;

    //private fields
    //runtime flags
    CURRENT_TRIG_FLOAT_TYPE *_curr_buf_w_ptr;
    CURRENT_TRIG_FLOAT_TYPE *_curr_buf_r_ptr;
    BitFlag8_t _curr_pending_event;
    //callback
    Callback_t _curr_dma_ht_cb;
    Callback_t _curr_dma_tc_cb;
}Device_CurrentTrig_t;


__STATIC_INLINE bool Device_CurrentTrig_IsEnabled(Device_CurrentTrig_t *self) 
{
    return HAL_Timer_IsEnabled(self->CurrentTrig_Timer);
}

void Device_CurrentTrig_Init(Device_CurrentTrig_t *self);
void Device_CurrentTrig_ConfigSet(Device_CurrentTrig_t *self,const Device_CurrentTrig_ConfigFile_t *config);
void Device_CurrentTrig_ConfigExport(const Device_CurrentTrig_t *self,Device_CurrentTrig_ConfigFile_t *config);
bool Device_CurrentTrig_IsConfigValid(Device_CurrentTrig_t *self,const Device_CurrentTrig_ConfigFile_t *config);
void Device_CurrentTrig_SetCallback(Device_CurrentTrig_t *self,
    Device_CurrentTrig_CallbackIdx_t idx,Callback_t *cb);
bool Device_CurrentTrig_Cmd(Device_CurrentTrig_t *self,bool en);
void Device_CurrentTrig_Service(Device_CurrentTrig_t *self);

#endif
