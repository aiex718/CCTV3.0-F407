#include "bsp/platform/platform_callbacks.h"
#include "bsp/platform/platform_inst.h"
#include "bsp/sys/callback.h"
#include "bsp/sys/sysctrl.h"


//Handlers
static void HardwareCtrl_WkupButton_ShortPress_Handler(void *sender, void *args, void *owner)
{
    Device_Buzzer_ShortBeep(Dev_Buzzer);
    Ethernetif_PrintIP(Dev_Ethernetif_Default);
}

static void HardwareCtrl_WkupButton_LongPress_Handler(void *sender, void *args, void *owner)
{
    if (Config_Storage_Erase(Dev_ConfigStorage))
    {
        Device_Buzzer_LongBeep(Dev_Buzzer);
        DBG_Serial_SafeMode(Peri_DBG_Serial, true);
        DBG_INFO("Button reset success, rebooting...\n");
        SysCtrl_ResetAfter(LONGPRESS_RESET_DELAY);
    }
    else
        DBG_ERROR("Button reset failed\n");
}

static void HardwareCtrl_CurrentTrig_Triggered_Handler(void *sender, void *args, void *owner)
{
    DBG_INFO("CurrentTrigger triggered!\n");
    Device_Buzzer_ShortBeep(Dev_Buzzer);
    if( Webhook_IsEnabled(App_Webhook_Triggered) &&
        Webhook_IsBusy(App_Webhook_Triggered)==false)
    {
        Webhook_Send(App_Webhook_Triggered);
        DBG_INFO("CurrentTrigger webhook sent!\n");
    }
}

#define CURRENT_TRIG_DISCONNECT_MSG_FILTER_CNT 50
static void HardwareCtrl_CurrentTrig_Disconnect_Handler(void *sender, void *args, void *owner)
{
    static uint8_t filter_cnt = 0;
    if((filter_cnt--) == 0)
    {
        filter_cnt=CURRENT_TRIG_DISCONNECT_MSG_FILTER_CNT;
        DBG_WARNING("CurrentTrigger current source disconnected\n");
    }
}

static void HardwareCtrl_CurrentTrig_Overload_Handler(void *sender, void *args, void *owner)
{
    DBG_WARNING("CurrentTrigger current overload!!!\n");
}

// callback instances
const Callback_t WkupButton_ShortPress_cb =
    {.func = HardwareCtrl_WkupButton_ShortPress_Handler};
const Callback_t WkupButton_LongPress_cb =
    {.func = HardwareCtrl_WkupButton_LongPress_Handler};
const Callback_t CurrentTrig_Triggered_cb =
    {.func = HardwareCtrl_CurrentTrig_Triggered_Handler};
const Callback_t CurrentTrig_Disconnect_cb =
    {.func = HardwareCtrl_CurrentTrig_Disconnect_Handler};
const Callback_t CurrentTrig_Overload_cb =
    {.func = HardwareCtrl_CurrentTrig_Overload_Handler};

void Platform_RegistCallbacks(void)
{
    Device_Button_SetCallback(Dev_Button_Wkup,
        BUTTON_CALLBACK_SHORT_PRESS, (Callback_t*)&WkupButton_ShortPress_cb);
    Device_Button_SetCallback(Dev_Button_Wkup,
        BUTTON_CALLBACK_LONG_PRESS, (Callback_t*)&WkupButton_LongPress_cb);
    Device_CurrentTrig_SetCallback(Dev_CurrentTrig,
        DEVICE_CURRENT_TRIG_CALLBACK_TRIGGERED, (Callback_t*)&CurrentTrig_Triggered_cb);
    Device_CurrentTrig_SetCallback(Dev_CurrentTrig,
        DEVICE_CURRENT_TRIG_CALLBACK_DISCONNECT, (Callback_t*)&CurrentTrig_Disconnect_cb);
    Device_CurrentTrig_SetCallback(Dev_CurrentTrig,
        DEVICE_CURRENT_TRIG_CALLBACK_OVERLOAD, (Callback_t*)&CurrentTrig_Overload_cb);
}
