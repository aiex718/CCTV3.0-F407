#include "device/usbotg_fs.h"
#include "usb_dcd_int.h"

void USBOTG_fs_Init(USBOTG_fs_t *self)
{
    USBD_Init(self->pcore, self->coreID, self->pDevice, self->class_cb, self->usr_cb);
}

void USBOTG_fs_IRQHandler(USBOTG_fs_t *self)
{
    USBD_OTG_ISR_Handler(self->pcore);
}
