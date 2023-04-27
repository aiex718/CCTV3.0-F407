#include "eth/apps/mjpeg/mjpegd_camera.h"
#include "eth/apps/mjpeg/mjpegd_frameproc.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"


#include "bsp/platform/device/dev_flashlight.h"

//private functions
static void Cam2640_NewFrame_Handler(void *sender, void *arg, void *owner);

void Mjpegd_Camera_Init(Mjpegd_Camera_t *cam,Mjpegd_t *mjpegd)
{
    //Camera init
	Device_CamOV2640_Init(cam->HwCam_Ov2640);

    cam->Ov2640_RecvRawFrame_cb.func = Cam2640_NewFrame_Handler;
    cam->Ov2640_RecvRawFrame_cb.invoke_cfg = INVOKE_IMMEDIATELY;
    cam->Ov2640_RecvRawFrame_cb.owner = mjpegd;
    Device_CamOV2640_SetCallback(cam->HwCam_Ov2640,
        CAMOV2640_CALLBACK_NEWFRAME,&cam->Ov2640_RecvRawFrame_cb);
}

// Let camera start to capture a frame, return false if any error occurs
// Caller MUST release frame when capture failed.
bool Mjpegd_Camera_DoSnap(Mjpegd_Camera_t *cam,Mjpegd_Frame_t *frame)
{
    if(Mjpegd_Camera_IsEnabled(cam) && frame!=NULL)
    {
        Device_CamOV2640_Status_t status;

        //check if old frame is not released, should never happen
        void* oldframe=cam->HwCam_Ov2640->pExtension;
        if(oldframe)
        {
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SEVERE,
                MJPEGD_DBG_ARG("DoSnap old frame not NULL, Memory Leaked %p\n",oldframe));
        }

        Mjpegd_Frame_Clear(frame);
        cam->HwCam_Ov2640->pExtension = frame;
        Device_CamOV2640_SetBuf(cam->HwCam_Ov2640,frame->payload,frame->payload_len);
        status = Device_CamOV2640_CaptureCmd(cam->HwCam_Ov2640,true);

        if(status==DEVICE_CAMOV2640_OK)
            return true;
        else
        {   
            //snap failed, maybe camera is not enabled
            //remove frame from camera, frame should be released by caller
            Device_CamOV2640_SetBuf(cam->HwCam_Ov2640,NULL,0);
            cam->HwCam_Ov2640->pExtension = NULL;
            
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                MJPEGD_DBG_ARG("Capture failed %d\n",status));
        }
    }
    return false;
}

void Mjpegd_Camera_Start(Mjpegd_Camera_t *cam)
{
	Device_FlashLight_Cmd(Dev_FlashLight_Top,true);
	Device_FlashLight_Cmd(Dev_FlashLight_Bottom,true);
    Device_CamOV2640_Cmd(cam->HwCam_Ov2640,true);

    LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
        MJPEGD_DBG_ARG("Camera_Start %p\n",cam));
}

void Mjpegd_Camera_Stop(Mjpegd_Camera_t *cam)
{
    Device_CamOV2640_Cmd(cam->HwCam_Ov2640,false);
    //stop capture will trigger dma tc callback
    //which will release frame
    Device_CamOV2640_CaptureCmd(cam->HwCam_Ov2640,false);
	Device_FlashLight_Cmd(Dev_FlashLight_Top,false);
	Device_FlashLight_Cmd(Dev_FlashLight_Bottom,false);

    LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_STATE,
        MJPEGD_DBG_ARG("Camera_Stop %p\n",cam));
}

u8_t Mjpegd_Camera_IsEnabled(Mjpegd_Camera_t *cam)
{
    return Device_CamOV2640_IsEnabled(cam->HwCam_Ov2640);
}

u8_t Mjpegd_Camera_IsSnapping(Mjpegd_Camera_t *cam)
{
    return Device_CamOV2640_IsCapturing(cam->HwCam_Ov2640);
}


/**
 * @brief   Callback invoked when new frame arrived.
 * @details New frame will be assigned to _pending_frame, waiting service call
 *          to process the raw frame.
 * @warning If original _pending_frame is not NULL, it will be released directly,
 *          which cause frame dropped, we dont buffer frame since all mjpegd client
 *          can reference to the same newest frame, buffer old frame make no sense.
 */
static void Cam2640_NewFrame_Handler(void *sender, void *arg, void *owner)
{
    Device_CamOV2640_t *cam2640 = (Device_CamOV2640_t*)sender;
    Mjpegd_t *mjpegd = (Mjpegd_t*)owner;
    Mjpegd_Frame_t *frame;

    BSP_UNUSED_ARG(arg);

    if(cam2640==NULL || mjpegd==NULL)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("NULL args\n"));
        return;
    }

    //remove frame from camera
    frame = (Mjpegd_Frame_t*)cam2640->pExtension;
    cam2640->pExtension=NULL;

    if(frame!=NULL)
        Mjpegd_Frame_SetLenAndTime(frame, cam2640->CamOV2640_Buffer_Len);
    else
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("NewFrame frame NULL\n"));
    }
    
    //clear buffer
    Device_CamOV2640_SetBuf(cam2640,NULL,0);
    
    if(Device_CamOV2640_IsEnabled(cam2640))
    {
        //return captured frame and get a new frame buffer
        frame=Mjpegd_FrameProc_NextFrame(mjpegd,frame);
        
        if(frame!=NULL)
        {
            //start next capture
            if(Mjpegd_Camera_DoSnap(mjpegd->Camera,frame)==false)
            {
                //failed to start capture, release frame
                Mjpegd_FrameProc_RecvBroken(mjpegd,frame);
                LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
                    MJPEGD_DBG_ARG("NewFrame DoSnap failed\n"));
            }
        }
        else
        {
            //no frame buffer available, try later
            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING, 
                MJPEGD_DBG_ARG("NewFrame no idle frame\n"));
        }
    }
    else
    {
        //camera is not enabled, just drop frame
        Mjpegd_FrameProc_RecvBroken(mjpegd,frame);
    }
}
