#include "bsp/platform/app/app_mjpegd.h"
#include "app/mjpegd/mjpegd_framepool.h"
#include "app/mjpegd/mjpegd_camera.h"

#include "bsp/platform/device/dev_cam_ov2640.h"

Mjpegd_Frame_t __mjpegd_frame_pool_hwbuf[MJPEGD_FRAMEPOOL_LEN] __HW_BUF_SECTION;

Mjpegd_t App_Mjpegd_Inst={
    .Port = 8080,
    .FramePool = __VAR_CAST_VAR(Mjpegd_FramePool_t){
        ._frames = __mjpegd_frame_pool_hwbuf,
    },
    .Camera = __VAR_CAST_VAR(Mjpegd_Camera_t){
		.Ov2640_RecvRawFrame_cb = NULL,
        .HwCam_Ov2640 = &Dev_Cam_OV2640_Inst,
    },
};
Mjpegd_t* App_Mjpegd = &App_Mjpegd_Inst;
