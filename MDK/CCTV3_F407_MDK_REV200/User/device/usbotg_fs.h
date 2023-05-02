#ifndef USBOTG_FS_H
#define USBOTG_FS_H

#include "bsp/platform/platform_defs.h"
#include "usbd_core.h"

typedef struct USBOTG_fs_s
{
    USB_OTG_CORE_HANDLE *pcore;
    USB_OTG_CORE_ID_TypeDef coreID;
    USBD_DEVICE *pDevice;        
    USBD_Class_cb_TypeDef *class_cb;
    USBD_Usr_cb_TypeDef *usr_cb;
} USBOTG_fs_t;

void USBOTG_fs_Init(USBOTG_fs_t *self);
void USBOTG_fs_IRQHandler(USBOTG_fs_t *self);

#endif
