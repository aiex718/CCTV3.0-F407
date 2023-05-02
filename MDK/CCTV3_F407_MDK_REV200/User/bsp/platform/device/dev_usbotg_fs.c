#include "bsp/platform/device/dev_usbotg_fs.h"

#include "usbd_usr.h"
#include "usbd_msc_core.h"
#include "usbd_desc.h"

USBOTG_fs_t Dev_USBOTG_fs_Inst = {
    .pcore = __VAR_CAST_VAR(USB_OTG_CORE_HANDLE){ 0 },
    .coreID = USB_OTG_FS_CORE_ID,
    .pDevice = &USR_desc,
    .class_cb = &USBD_MSC_cb,
    .usr_cb = &USR_cb,
};

USBOTG_fs_t *Dev_USBOTG_fs = &Dev_USBOTG_fs_Inst;
