#include "bsp/platform/platform_callbacks.h"
#include "bsp/platform/platform_inst.h"
#include "bsp/sys/callback.h"

//Handlers
static void HardwareCtrl_WkupButton_ShortPress_Handler(void *sender, void *args, void *owner)
{
    Device_Buzzer_ShortBeep(Dev_Buzzer);
    // TODO:Print ip
}

static void HardwareCtrl_WkupButton_LongPress_Handler(void *sender, void *args, void *owner)
{
    if (Config_Storage_Erase(Dev_ConfigStorage))
    {
        Device_Buzzer_LongBeep(Dev_Buzzer);
        DBG_INFO("Button reset success, rebooting...\n");
        DBG_Serial_Flush(Peri_DBG_Serial);
        // TODO:DelayReset SysCtrl_DelayReset(3000);
    }
    else
        DBG_ERROR("Button reset failed\n");
}

static void HardwareCtrl_CurrentTrig_Triggered_Handler(void *sender, void *args, void *owner)
{
    if( Webhook_IsEnabled(Current_Trig_Webhook) &&
        Webhook_IsBusy(Current_Trig_Webhook)==false)
    {
        Webhook_Send(Current_Trig_Webhook);
    }
}

// callback instances
const Callback_t WkupButton_ShortPress_cb =
    {.func = HardwareCtrl_WkupButton_ShortPress_Handler};
const Callback_t WkupButton_LongPress_cb =
    {.func = HardwareCtrl_WkupButton_LongPress_Handler};
const Callback_t CurrentTrig_Triggered_cb =
    {.func = HardwareCtrl_CurrentTrig_Triggered_Handler};

void Platform_RegistCallbacks(void)
{
    Device_Button_SetCallback(Dev_Button_Wkup,
        BUTTON_CALLBACK_SHORT_PRESS, (Callback_t*)&WkupButton_ShortPress_cb);
    Device_Button_SetCallback(Dev_Button_Wkup,
        BUTTON_CALLBACK_LONG_PRESS, (Callback_t*)&WkupButton_LongPress_cb);
    Device_CurrentTrig_SetCallback(Dev_CurrentTrig,
        DEVICE_CURRENT_TRIG_CALLBACK_TRIGGERED, (Callback_t*)&CurrentTrig_Triggered_cb);
}
