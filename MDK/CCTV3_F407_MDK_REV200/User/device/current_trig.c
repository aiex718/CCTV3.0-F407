#include "device/current_trig.h"

#include "bsp/sys/dbg_serial.h"

//Current(mA) to ADC raw value convertion 

// ADC_raw = I(A) * CURRENT_TRIG_SENSING_RESISTOR_VAL * ADC_resolution / Vref
#define _MA_TO_ADC_RAW_VAL(ma) ((CURRENT_TRIG_ADC_VAL_TYPE)\
    (ma)*CURRENT_TRIG_SENSING_RESISTOR_VAL*CURRENT_TRIG_ADC_RESOLUTION/(CURRENT_TRIG_ADC_VREF*1000))

// Current(A) = (ADC_Value * Vref) / (ADC_Resolution * Resistor) , mul 1000 to get mA value
#define _ADC_RAW_VAL_TO_MA(ma) (((CURRENT_TRIG_FLOAT_TYPE)*data) * \
    ((CURRENT_TRIG_ADC_VREF * 1000) / (CURRENT_TRIG_ADC_RESOLUTION * CURRENT_TRIG_SENSING_RESISTOR_VAL)))

//Same as above but with divider to standarize the value
#define _ADC_RAW_VAL_TO_MA_DIV(ma,div) (((CURRENT_TRIG_FLOAT_TYPE)*data) * \
    ((CURRENT_TRIG_ADC_VREF * 1000 / (div)) / (CURRENT_TRIG_ADC_RESOLUTION * CURRENT_TRIG_SENSING_RESISTOR_VAL)))

//private functions
static float stddev(float data[], uint16_t len);
static float mean(float data[], uint16_t len);
static void Device_CurrentTrig_ProcPendingEvent(Device_CurrentTrig_t *self);
static void Device_CurrentTrig_ProcADCRawVal(Device_CurrentTrig_t *self,const CURRENT_TRIG_ADC_VAL_TYPE* data,uint16_t len);
static void Device_CurrentTrig_CheckADCThres(Device_CurrentTrig_t *self,const CURRENT_TRIG_ADC_VAL_TYPE* data,uint16_t len);
static void Device_CurrentTrig_CheckRecvLen(Device_CurrentTrig_t *self);
static bool Device_CurrentTrig_CheckPeak(Device_CurrentTrig_t *self);
static void Device_CurrentTrig_Read_Buf(Device_CurrentTrig_t *self, CURRENT_TRIG_FLOAT_TYPE *dst,uint16_t len);


//private events
typedef enum {
    CURRENTTRIG_EVENT_DMA_HT    = 0 ,
    CURRENTTRIG_EVENT_DMA_TC        ,
}_Device_CurrentTrig_Pending_Event_t;

void Device_CurrentTrig_DmaHT_Callback(void *sender, void *arg, void *owner)
{
    Device_CurrentTrig_t *trig = (Device_CurrentTrig_t *)owner;

    if(BitFlag_IsIdxSet(trig->_curr_pending_event,CURRENTTRIG_EVENT_DMA_HT))
        DBG_WARNING("DmaHT already pending, too slow?\n");
    else
        BitFlag_SetIdx(trig->_curr_pending_event,CURRENTTRIG_EVENT_DMA_HT);

#if CURRENT_TRIG_DEBUG_PRINT_ADC_VALUE
    {
        DBG_INFO("DmaHT_Callback: ");
        CURRENT_TRIG_ADC_VAL_TYPE *start = trig->CurrentTrig_ADC_Buf;
        CURRENT_TRIG_ADC_VAL_TYPE *end = start + (trig->CurrentTrig_ADC_Buf_Len>>1);
        while (start < end)
            DBG_PRINTF("%d ", *start++);
        DBG_PRINTF("\n");
    }
#endif
}

void Device_CurrentTrig_DmaTC_Callback(void *sender, void *arg, void *owner)
{
    Device_CurrentTrig_t *trig = (Device_CurrentTrig_t *)owner;

    if(BitFlag_IsIdxSet(trig->_curr_pending_event,CURRENTTRIG_EVENT_DMA_TC))
        DBG_WARNING("DMA_TC already pending, too slow?\n");
    else
        BitFlag_SetIdx(trig->_curr_pending_event,CURRENTTRIG_EVENT_DMA_TC);

#if CURRENT_TRIG_DEBUG_PRINT_ADC_VALUE
    {
        DBG_INFO("DmaTC_Callback: ");
        CURRENT_TRIG_ADC_VAL_TYPE *start = trig->CurrentTrig_ADC_Buf + (trig->CurrentTrig_ADC_Buf_Len>>1);
        CURRENT_TRIG_ADC_VAL_TYPE *end = start + (trig->CurrentTrig_ADC_Buf_Len>>1);
        while (start < end)
            DBG_PRINTF("%d ", *start++);
        DBG_PRINTF("\n");
    }
#endif

}

void Device_CurrentTrig_Init(Device_CurrentTrig_t *self)
{
    BSP_ARR_CLEAR(self->CurrentTrig_Callbacks);

    HAL_Timer_Init(self->CurrentTrig_Timer);
    HAL_ADC_Init(self->CurrentTrig_ADC);
    HAL_DMA_Init(self->CurrentTrig_DMA);

    //flags is cleared when cmd enabled
    self->_curr_dma_ht_cb.func = Device_CurrentTrig_DmaHT_Callback;
    self->_curr_dma_ht_cb.invoke_cfg = INVOKE_IMMEDIATELY;
    self->_curr_dma_ht_cb.owner = self;
    HAL_DMA_SetCallback(self->CurrentTrig_DMA, DMA_CALLBACK_IRQ_HT, &self->_curr_dma_ht_cb);
    self->_curr_dma_tc_cb.func = Device_CurrentTrig_DmaTC_Callback;
    self->_curr_dma_tc_cb.invoke_cfg = INVOKE_IMMEDIATELY;
    self->_curr_dma_tc_cb.owner = self;
    HAL_DMA_SetCallback(self->CurrentTrig_DMA, DMA_CALLBACK_IRQ_TC, &self->_curr_dma_tc_cb);
}

void Device_CurrentTrig_ConfigSet(Device_CurrentTrig_t *self,const Device_CurrentTrig_ConfigFile_t *config)
{
    self->CurrentTrig_Enable = config->CurrentTrig_Enable;
    self->CurrentTrig_Disconnect_Thres_mA = config->CurrentTrig_Disconnect_Thres_mA;
    self->CurrentTrig_Overload_Thres_mA = config->CurrentTrig_Overload_Thres_mA;
    self->CurrentTrig_PeakThreshold = config->CurrentTrig_PeakThreshold_1000x/1000.0F;
    self->CurrentTrig_PeakInfluence = config->CurrentTrig_PeakInfluence_1000x/1000.0F;
}

void Device_CurrentTrig_ConfigExport(const Device_CurrentTrig_t *self,Device_CurrentTrig_ConfigFile_t *config)
{
    config->CurrentTrig_Enable = self->CurrentTrig_Enable;
    config->CurrentTrig_Disconnect_Thres_mA = self->CurrentTrig_Disconnect_Thres_mA;
    config->CurrentTrig_Overload_Thres_mA = self->CurrentTrig_Overload_Thres_mA;
    config->CurrentTrig_PeakThreshold_1000x = self->CurrentTrig_PeakThreshold*1000;
    config->CurrentTrig_PeakInfluence_1000x = self->CurrentTrig_PeakInfluence*1000;
}

bool Device_CurrentTrig_IsConfigValid(Device_CurrentTrig_t *self,const Device_CurrentTrig_ConfigFile_t *config)
{
    if(config == NULL || config->CurrentTrig_Enable > 1)
        return false;
    else if (config->CurrentTrig_Enable == false)
        return true;
    else
        return ( config->CurrentTrig_Disconnect_Thres_mA <= 8 &&
        config->CurrentTrig_Overload_Thres_mA >= 16 && config->CurrentTrig_Overload_Thres_mA <= 24 &&
        config->CurrentTrig_PeakThreshold_1000x > 0 && config->CurrentTrig_PeakThreshold_1000x < (uint16_t) -1 &&
        config->CurrentTrig_PeakInfluence_1000x <=1000);
}


void Device_CurrentTrig_SetCallback(Device_CurrentTrig_t *self, Device_CurrentTrig_CallbackIdx_t idx, Callback_t *cb)
{
    if (idx < __NOT_CALLBACK_CURRENTTRIG_MAX)
        self->CurrentTrig_Callbacks[idx] = cb;
}

bool Device_CurrentTrig_Cmd(Device_CurrentTrig_t *self, bool en)
{
    if (en)
    {
        if (self->CurrentTrig_ADC_Buf == NULL || self->CurrentTrig_ADC_Buf_Len == 0 ||
            self->CurrentTrig_Val_Buf == NULL || self->CurrentTrig_Val_Buf_Len == 0 )
            return false;

        HAL_DMA_SetPeriphAddr(self->CurrentTrig_DMA,
                              (uint32_t)HAL_ADC_GetDmaAddr(self->CurrentTrig_ADC));
        HAL_DMA_SetMemAddr(self->CurrentTrig_DMA, (uint32_t)self->CurrentTrig_ADC_Buf);
        HAL_DMA_SetNumOfData(self->CurrentTrig_DMA, self->CurrentTrig_ADC_Buf_Len);

#if CURRENT_TRIG_DEBUG
    DBG_INFO("CurrentTrig started\n");
    DBG_INFO("ADCBuf 0x%p len %d\n", self->CurrentTrig_ADC_Buf, self->CurrentTrig_ADC_Buf_Len);
    DBG_INFO("Buf 0x%p len %d, end at 0x%p\n", self->CurrentTrig_Val_Buf, self->CurrentTrig_Val_Buf_Len,
        self->CurrentTrig_Val_Buf+self->CurrentTrig_Val_Buf_Len);
#endif
    }

    //clear runtime stat
    self->_curr_buf_r_ptr = self->CurrentTrig_Val_Buf;
    self->_curr_buf_w_ptr = self->CurrentTrig_Val_Buf;
    BitFlag_Clear(self->_curr_pending_event);

    //cmd to sub modules
    HAL_Timer_Cmd(self->CurrentTrig_Timer, en);
    HAL_DMA_Cmd(self->CurrentTrig_DMA, en);
    HAL_ADC_DMACmd(self->CurrentTrig_ADC, en);
    HAL_ADC_Cmd(self->CurrentTrig_ADC, en);

    return true;
}

void Device_CurrentTrig_Service(Device_CurrentTrig_t *self)
{
    Device_CurrentTrig_ProcPendingEvent(self);
    Device_CurrentTrig_CheckRecvLen(self);
}   

static void Device_CurrentTrig_ProcPendingEvent(Device_CurrentTrig_t *self)
{
    uint8_t event_idx;
    CURRENT_TRIG_ADC_VAL_TYPE *adc_buf_ptr=NULL;
    uint16_t read_len=0;

    while(self->_curr_pending_event)
    {
        //check if DMA_HT or DMA_TC event occoured
        event_idx = BitFlag_BinToIdx(self->_curr_pending_event);

        if(event_idx == CURRENTTRIG_EVENT_DMA_HT)
        {
            adc_buf_ptr = self->CurrentTrig_ADC_Buf;
            read_len = self->CurrentTrig_ADC_Buf_Len>>1;
        }
        else if(event_idx == CURRENTTRIG_EVENT_DMA_TC)
        {
            adc_buf_ptr = self->CurrentTrig_ADC_Buf+(self->CurrentTrig_ADC_Buf_Len>>1);
            read_len = self->CurrentTrig_ADC_Buf_Len>>1;
        }

        if(adc_buf_ptr != NULL && read_len)
        {
            Device_CurrentTrig_ProcADCRawVal(self,adc_buf_ptr,read_len);
            Device_CurrentTrig_CheckADCThres(self,adc_buf_ptr,read_len);
        }
        
        BitFlag_ClearIdx(self->_curr_pending_event,event_idx);
    }
}

//Convert ADC raw value to standarized value, write to self->CurrentTrig_Val_Buf.
static void Device_CurrentTrig_ProcADCRawVal(Device_CurrentTrig_t *self,const CURRENT_TRIG_ADC_VAL_TYPE* data,uint16_t len)
{
    const CURRENT_TRIG_FLOAT_TYPE *buf_end = self->CurrentTrig_Val_Buf + self->CurrentTrig_Val_Buf_Len;
    const CURRENT_TRIG_ADC_VAL_TYPE *data_end = data + len;

    while(data!=data_end)
    {
        //div CURRENT_TRIG_MAX_CURRENT_MA to standarize data from 0 to 1.00
        *self->_curr_buf_w_ptr= _ADC_RAW_VAL_TO_MA_DIV(*data,CURRENT_TRIG_MAX_CURRENT_MA);

        ++self->_curr_buf_w_ptr;
        ++data;

        if(self->_curr_buf_w_ptr==buf_end)
            self->_curr_buf_w_ptr=self->CurrentTrig_Val_Buf;
    }
}

//check if adc vaule exceed threshold, trigger DISCONNECT or OVERLOAD callback if needed
static void Device_CurrentTrig_CheckADCThres(Device_CurrentTrig_t *self,const CURRENT_TRIG_ADC_VAL_TYPE* data,uint16_t len)
{
    const CURRENT_TRIG_ADC_VAL_TYPE *data_end = data + len;
    const CURRENT_TRIG_ADC_VAL_TYPE low_thres = _MA_TO_ADC_RAW_VAL(self->CurrentTrig_Disconnect_Thres_mA);
    const CURRENT_TRIG_ADC_VAL_TYPE high_thres = _MA_TO_ADC_RAW_VAL(self->CurrentTrig_Overload_Thres_mA);
    bool disconnect = false, overload = false;

    while(data!=data_end)
    {
        if(*data <= low_thres)
        {
#if CURRENT_TRIG_DEBUG
            DBG_INFO("ISEN disconnect: %d <= %d\n", *data,low_thres);
#endif
            disconnect = true;
        }
        else if(*data >= high_thres)
        {
#if CURRENT_TRIG_DEBUG
            DBG_INFO("ISEN overload: %d >= %d\n", *data, high_thres);
#endif
            overload = true;
        }
        ++data;
    }


    if(disconnect)
    {
        Callback_Invoke_Idx(
            self,NULL,self->CurrentTrig_Callbacks, 
            DEVICE_CURRENT_TRIG_CALLBACK_DISCONNECT);
        DBG_WARNING("ISEN disconnect!\n");
    }

    if(overload)
    {
        Callback_Invoke_Idx(
            self,NULL,self->CurrentTrig_Callbacks, 
            DEVICE_CURRENT_TRIG_CALLBACK_OVERLOAD);
        DBG_WARNING("ISEN overloaded!\n");
    }
}

//check if received enough data to scan for peak value
static void Device_CurrentTrig_CheckRecvLen(Device_CurrentTrig_t *self)
{
    uint16_t recv_len;
    
    if(self->_curr_buf_w_ptr >= self->_curr_buf_r_ptr)
        recv_len = self->_curr_buf_w_ptr - self->_curr_buf_r_ptr;
    else
        recv_len = self->CurrentTrig_Val_Buf_Len - (self->_curr_buf_r_ptr - self->_curr_buf_w_ptr);

    if(recv_len >= CURRENT_TRIG_SCAN_WINDOW)
    {
#if CURRENT_TRIG_DEBUG
        DBG_INFO("recv_len %d>=%d ,buf w_ptr 0x%p r_ptr 0x%p\n",
            recv_len,CURRENT_TRIG_SCAN_WINDOW,self->_curr_buf_w_ptr,self->_curr_buf_r_ptr);
#endif
        if(Device_CurrentTrig_CheckPeak(self))
        {
#if CURRENT_TRIG_DEBUG_PRINT_EVENT
            DBG_INFO("Triggered!\n");
#endif
            Callback_Invoke_Idx(
                self,NULL,self->CurrentTrig_Callbacks, 
                DEVICE_CURRENT_TRIG_CALLBACK_TRIGGERED);
        }
    }
}

static bool Device_CurrentTrig_CheckPeak(Device_CurrentTrig_t *self)
{
    bool result = false;
    uint16_t i, lag = CURRENT_TRIG_SCAN_WINDOW / 2;

    CURRENT_TRIG_FLOAT_TYPE y[CURRENT_TRIG_SCAN_WINDOW];
    CURRENT_TRIG_FLOAT_TYPE filteredY[CURRENT_TRIG_SCAN_WINDOW];
    CURRENT_TRIG_FLOAT_TYPE avgFilter[CURRENT_TRIG_SCAN_WINDOW];
    CURRENT_TRIG_FLOAT_TYPE stdFilter[CURRENT_TRIG_SCAN_WINDOW];

    CURRENT_TRIG_FLOAT_TYPE y_avg_diff;
    const CURRENT_TRIG_FLOAT_TYPE threshold = self->CurrentTrig_PeakThreshold;
    const CURRENT_TRIG_FLOAT_TYPE influence = self->CurrentTrig_PeakInfluence;


#if CURRENT_TRIG_DEBUG
    bool have_edge = false;
    DBG_INFO("Checking peak from 0x%p len %d \n", self->_curr_buf_r_ptr, CURRENT_TRIG_SCAN_WINDOW);
#endif

#if CURRENT_TRIG_DEBUG_PRINT_SIGNAL
    int8_t signals[CURRENT_TRIG_SCAN_WINDOW] = {0};
#endif

    //read and copy data to local y[]
    Device_CurrentTrig_Read_Buf(self, y, BSP_ARR_LEN(y));
    //copy data to filteredY[]
    BSP_MEMCPY(filteredY, y, sizeof(float) * BSP_ARR_LEN(y));
    avgFilter[lag - 1] = mean(y, lag);
    stdFilter[lag - 1] = stddev(y, lag);

    
    for (i = lag; i < CURRENT_TRIG_SCAN_WINDOW; i++)
    {
        y_avg_diff = BSP_ABS(y[i] - avgFilter[i - 1]);
        if (y_avg_diff >= CURRENT_TRIG_DIFF_MIN && y_avg_diff > threshold * stdFilter[i - 1])
        {
#if CURRENT_TRIG_DEBUG
            DBG_INFO("Edge at %d ,%f > %f\n",i, y_avg_diff, threshold * stdFilter[i - 1]);
#endif
            if (y[i] > avgFilter[i - 1])
            {
                
#if CURRENT_TRIG_DEBUG
                have_edge = true;
                DBG_INFO("Peak at %d ,%2.2f > %2.2f\n", i, y[i], avgFilter[i - 1]);
#endif

                result = true;
#if CURRENT_TRIG_DEBUG_PRINT_SIGNAL
                signals[i] = 1;
#else 
                break;
#endif
            }
            else
            {
#if CURRENT_TRIG_DEBUG
                have_edge = true;
                DBG_INFO("Drop at %d ,%2.2f <= %2.2f\n", i, y[i], avgFilter[i - 1]);
#endif

#if CURRENT_TRIG_DEBUG_PRINT_SIGNAL
                signals[i] = -1;
#endif
            }

            filteredY[i] = influence * y[i] + (1 - influence) * filteredY[i - 1];
        }
        avgFilter[i] = mean(filteredY + i - lag, lag);
        stdFilter[i] = stddev(filteredY + i - lag, lag);
    }

#if CURRENT_TRIG_DEBUG_PRINT_SIGNAL && CURRENT_TRIG_DEBUG
    if(have_edge)
    {
        DBG_PRINTF("Peak at ");
        for (i = 0; i < BSP_ARR_LEN(signals); i++)
        {
            if (signals[i] == 1)
                DBG_PRINTF("%d, ", i);
        }
        DBG_PRINTF("\n");

        DBG_PRINTF("Drop at ");
        for (i = 0; i < BSP_ARR_LEN(signals); i++)
        {
            if (signals[i] == -1)
                DBG_PRINTF("%d, ", i);
        }
        DBG_PRINTF("\n");

        DBG_PRINTF("Data     0     1     2     3     4     5     6     7     8     9\n");
        for (i = 0; i < BSP_ARR_LEN(y); i += 10)
        {
            uint16_t j;
            DBG_PRINTF("%3d:  ", i);
            for (j = 0; j < 10; j++)
            {
                DBG_PRINTF("%4.2f  ", y[i + j]);
            }
            DBG_PRINTF("\n");
        }
        DBG_PRINTF("\n");
    }
#endif

    return result;
}


static void Device_CurrentTrig_Read_Buf(Device_CurrentTrig_t *self, CURRENT_TRIG_FLOAT_TYPE *dst,uint16_t len)
{
    const float *buf_end = self->CurrentTrig_Val_Buf + self->CurrentTrig_Val_Buf_Len;
    float *r_ptr = self->_curr_buf_r_ptr;
    uint16_t partial_len, copied_len = 0;

    while(len)
    {
        partial_len = BSP_MIN(len, buf_end - r_ptr);

        if(r_ptr + partial_len > buf_end)
        {
            DBG_ERROR("Buf copy over edge, r_ptr 0x%p partial_len %d buf_end 0x%p\n",
                    r_ptr, partial_len, buf_end);
        }
        BSP_MEMCPY(dst + copied_len, r_ptr, sizeof(float) * partial_len);

        len -= partial_len;
        copied_len += partial_len;

        r_ptr += partial_len;
        if(r_ptr >= buf_end)
            r_ptr -= self->CurrentTrig_Val_Buf_Len;
    }

    r_ptr -= CURRENT_TRIG_SCAN_ROLLBACK;
    if(r_ptr < self->CurrentTrig_Val_Buf)
        r_ptr += self->CurrentTrig_Val_Buf_Len;

    self->_curr_buf_r_ptr = r_ptr;
}



static float mean(float data[], uint16_t len)
{
    float sum = 0.0, mean = 0.0;

    uint16_t i;
    for (i = 0; i < len; ++i)
    {
        sum += data[i];
    }

    mean = sum / len;
    return mean;
}

static float stddev(float data[], uint16_t len)
{
    float the_mean = mean(data, len);
    float standardDeviation = 0.0;

    uint16_t i;
    for (i = 0; i < len; ++i)
    {
        standardDeviation += pow(data[i] - the_mean, 2);
    }

    return sqrt(standardDeviation / len);
}
