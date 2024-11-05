#include "app/uartcmd/uartcmd.h"

#include "bsp/platform/platform_inst.h"
#include "bsp/sys/sysctrl.h"
#include "bsp/sys/mem_guard.h"
#include "bsp/sys/perfmon.h"


void UartCmd_Service(UartCmd_t *self)
{
    uint8_t *rxcmd = self->UartCmd_RxBuf;
    if(DBG_Serial_ReadLine(Peri_DBG_Serial,rxcmd,sizeof(self->UartCmd_RxBuf)))
    {
        if(BSP_STRCMP((char*)rxcmd,"hello")==0)
            DBG_INFO("Hello there\n");
        else if(BSP_STRCMP((char*)rxcmd,"time")==0)
        {
            HAL_RTC_PrintTime(Peri_RTC);
        }
        else if(BSP_STRCMP((char*)rxcmd,"stack")==0)
        {
            DBG_INFO("Stack usage 0x%x\n",Mem_Guard_GetStackDepth());
        }
        else if(BSP_STRCMP((char*)rxcmd,"format")==0)
        {
            if(FileSys_Format(App_FileSys,""))
                DBG_INFO("FileSys_Format success\n");
            else
                DBG_ERROR("FileSys_Format failed\n");
        }
        else if(BSP_STRCMP((char*)rxcmd,"reset")==0)
        {
            if(Config_Storage_Erase(Dev_ConfigStorage))
            {
                DBG_Serial_SafeMode(Peri_DBG_Serial,true);
                DBG_INFO("System reset success, rebooting...\n");
                SysCtrl_Reset();
            }
            else
                DBG_ERROR("System reset failed\n");
        }
        else if(BSP_STRCMP((char*)rxcmd,"reboot")==0)
        {
            DBG_Serial_SafeMode(Peri_DBG_Serial,true);
            DBG_INFO("System rebooting...\n");
            SysCtrl_Reset();
        }
        else if(BSP_STRCMP((char*)rxcmd,"beep")==0)
        {
            Device_Buzzer_ShortBeep(Dev_Buzzer);
        }
        else if(BSP_STRCMP((char*)rxcmd,"ip")==0)
        {
            Ethernetif_PrintIP(Dev_Ethernetif_Default);
        }
        else if(BSP_STRCMP((char*)rxcmd,"dhcp")==0)
        {
            const Ethernetif_ConfigFile_t *eth_conf = (const Ethernetif_ConfigFile_t *)
                Config_Storage_Read(Dev_ConfigStorage,Dev_Ethernetif_Default,NULL);

            if(eth_conf)
            {
                Ethernetif_ConfigFile_t eth_conf_copy;
                eth_conf_copy = *eth_conf;  
                eth_conf_copy.Netif_Config_DHCP_Enable = !eth_conf_copy.Netif_Config_DHCP_Enable;

                Config_Storage_Write(Dev_ConfigStorage,Dev_Ethernetif_Default,&eth_conf_copy);
                Config_Storage_Commit(Dev_ConfigStorage);

                DBG_Serial_SafeMode(Peri_DBG_Serial,true);
                DBG_INFO("DHCP is now %s\n",eth_conf->Netif_Config_DHCP_Enable?"enabled":"disabled");
                DBG_INFO("System rebooting...\n");
                SysCtrl_Reset();
            }
            else
            {
                DBG_ERROR("Failed to read ethernet config\n");
            }
        } 
        else if(BSP_STRCMP((char*)rxcmd,"fault")==0)
        {
            SysCtrl_RaiseHardFault();
        }
        else if(BSP_STRCMP((char*)rxcmd,"loopfreq")==0)
        {
            DBG_INFO("LoopFreq:%d\n",PerfMon_GetLoopFreq());
        }
        else if (BSP_STRCMP((char*)rxcmd,"help")==0)
        {
            DBG_INFO("\n"
            "Commands Available:\n"
            "hello - say hello\n"
            "time - print time\n"
            "stack - print stack usage\n"
            "format - format disk\n"
            "reset - factory reset system\n"
            "reboot - reboot system\n"
            "beep - beep\n"
            "ip - print ip\n"
            "dhcp - toggle dhcp\n"
            "fault - raise hard fault\n"
            "loopfreq - print main loop frequency\n"
            "help - print this help\n");
        }
        else
        {
            DBG_ERROR("Unknown command:%s\n",rxcmd);
        }
        
        BSP_MEMSET(self->UartCmd_RxBuf,0,sizeof(self->UartCmd_RxBuf));
    }
}

void UartCmd_PrintHelpMsg(void)
{
    DBG_INFO("Input 'help' for command list\n");
}
