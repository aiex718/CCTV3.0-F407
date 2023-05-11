#include "app/httpd/webapi.h"
#include "bsp/platform/platform_inst.h"
#include "app/httpd/http_builder.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/sys/sysctrl.h"

#include "lwip/timeouts.h"
#include "lwip/apps/fs.h"
#include "lwip/ip_addr.h"
#if LWIP_DNS
#include "lwip/dns.h"
#endif

const char *const Webapi_Enter_Point = "/api";

typedef enum
{
    WEBAPI_OK = 0,
    WEBAPI_ERR_FAILED,
    WEBAPI_ERR_BAD_ACT,
    WEBAPI_ERR_NOTFOUND,
} WebApi_Result_t;

typedef struct
{
    char *cmd_str;
    WebApi_Result_t (*FuncPtr)(struct fs_file *file,
                               const char *uri, int iNumParams, char **pcParam, char **pcValue);
} Webapi_Cmd_FuncPtr_Map_t;

static WebApi_Result_t Webapi_Uptime_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue);
static WebApi_Result_t Webapi_IP_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue);
static WebApi_Result_t Webapi_DHCP_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue);
static WebApi_Result_t Webapi_SNTP_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue);
static WebApi_Result_t Webapi_Current_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue);
static WebApi_Result_t Webapi_Light_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue);
static WebApi_Result_t Webapi_Camera_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue);
static WebApi_Result_t Webapi_Reboot_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue);

static const Webapi_Cmd_FuncPtr_Map_t Webapi_cmds[] =
{
    {"uptime", Webapi_Uptime_Handler},
    {"ip", Webapi_IP_Handler},
    {"dhcp", Webapi_DHCP_Handler},
    {"sntp", Webapi_SNTP_Handler},
    {"current", Webapi_Current_Handler},
    {"light", Webapi_Light_Handler},
    {"camera", Webapi_Camera_Handler},
    {"reboot", Webapi_Reboot_Handler},
};

const char ACT_STR[] = "act", CMD_STR[] = "cmd", GET_STR[] = "get", SET_STR[] = "set";
const char REBOOT_PW[] = "uuddlrlrab";
const char BAD_ACT_MESSAGE[] = "Act not match.";
const char VALUE_STR[] = "value", TRUE_STR[] = "true", FALSE_STR[] = "false";
const char RESULT_TRUE_JSON[] = "{\"result\":\"true\"}";
const char RESULT_FALSE_JSON[] = "{\"result\":\"false\"}";

__STATIC_INLINE WebApi_Result_t Default_Success_Response(struct fs_file *file)
{
    HttpBuilder_BuildResponse(file, HTTP_RESPONSE_200_OK);
    HttpBuilder_Insert(file, RESULT_TRUE_JSON);
    return WEBAPI_OK;
}

__STATIC_INLINE WebApi_Result_t Default_Fail_Response(struct fs_file *file)
{
    HttpBuilder_BuildResponse(file, HTTP_RESPONSE_400_BAD_REQUEST);
    HttpBuilder_Insert(file, RESULT_FALSE_JSON);
    return WEBAPI_ERR_FAILED;
}

__STATIC_INLINE WebApi_Result_t Act_Fail_Response(struct fs_file *file)
{
    HttpBuilder_BuildResponse(file, HTTP_RESPONSE_400_BAD_REQUEST);
    HttpBuilder_Insert(file, BAD_ACT_MESSAGE);
    return WEBAPI_ERR_BAD_ACT;
}

static char *ReadParam(const char *name, int iNumParams, char **pcParam, char **pcValue)
{
    while (--iNumParams >= 0)
    {
        if (BSP_STRCMP(pcParam[iNumParams], name) == 0)
            return pcValue[iNumParams];
    }
    return NULL;
}

#if 1 /* WebApi Handlers */
static WebApi_Result_t Webapi_Uptime_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    HttpBuilder_BuildResponse(file, HTTP_RESPONSE_200_OK);
    HttpBuilder_printf(file, "{\"uptime\":%u,", sys_now());
    HttpBuilder_printf(file, "\"nowtime\":%u}", (uint32_t)HAL_RTC_GetTime(Peri_RTC));
    return WEBAPI_OK;
}

static WebApi_Result_t Webapi_IP_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    char *act = ReadParam(ACT_STR, iNumParams, pcParam, pcValue);
    void *obj = Dev_Ethernetif_Default;
    const Ethernetif_ConfigFile_t *config = (Ethernetif_ConfigFile_t *)
        Config_Storage_Read(Dev_ConfigStorage, obj, NULL);

    if (config == NULL)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_503_UNAVAILABLE);
        return WEBAPI_ERR_FAILED;
    }
    else if (BSP_STRCMP((const char *)act, GET_STR) == 0)
    {
        struct netif *_netif = &Dev_Ethernetif_Default->_netif;
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_200_OK);

        HttpBuilder_printf(file, "{\"ip\":\"%s\",", ip4addr_ntoa(&(_netif->ip_addr)));
        HttpBuilder_printf(file, "\"netmask\":\"%s\",", ip4addr_ntoa(&(_netif->netmask)));
        HttpBuilder_printf(file, "\"gateway\":\"%s\",", ip4addr_ntoa(&(_netif->gw)));
#if LWIP_DNS
        HttpBuilder_printf(file, "\"dns0\":\"%s\",", ip4addr_ntoa(dns_getserver(0)));
        HttpBuilder_printf(file, "\"dns1\":\"%s\"", ip4addr_ntoa(dns_getserver(1)));
#endif
        HttpBuilder_printf(file, "}", ip4addr_ntoa(&(_netif->gw)));

        return WEBAPI_OK;
    }
    else if (BSP_STRCMP((const char *)act, SET_STR) == 0)
    {
        Ethernetif_ConfigFile_t new_config = *config; // copy original config

        ip4_addr_t temp;
        char *ip = ReadParam("ip", iNumParams, pcParam, pcValue);
        char *mask = ReadParam("mask", iNumParams, pcParam, pcValue);
        char *gw = ReadParam("gw", iNumParams, pcParam, pcValue);
        char *dns0 = ReadParam("dns0", iNumParams, pcParam, pcValue);
        char *dns1 = ReadParam("dns1", iNumParams, pcParam, pcValue);

        // write to new config
        if (ip && ip4addr_aton(ip, &temp))
        {
            BSP_STRNCPY(new_config.Netif_Config_IP, ip, sizeof(new_config.Netif_Config_IP));
            BSP_ARR_STREND(new_config.Netif_Config_IP);
            new_config.Netif_Config_DHCP_Enable = false;
        }
        if (mask && ip4addr_aton(mask, &temp))
        {
            BSP_STRNCPY(new_config.Netif_Config_Mask, mask, sizeof(new_config.Netif_Config_Mask));
            BSP_ARR_STREND(new_config.Netif_Config_Mask);
        }
        if (gw && ip4addr_aton(gw, &temp))
        {
            BSP_STRNCPY(new_config.Netif_Config_Gateway, gw, sizeof(new_config.Netif_Config_Gateway));
            BSP_ARR_STREND(new_config.Netif_Config_Gateway);
        }
#if LWIP_DNS
        if (dns0 && ip4addr_aton(dns0, &temp))
        {
            BSP_STRNCPY(new_config.Netif_Config_DNS0, dns0, sizeof(new_config.Netif_Config_DNS0));
            BSP_ARR_STREND(new_config.Netif_Config_DNS0);
        }
        if (dns1 && ip4addr_aton(dns1, &temp))
        {
            BSP_STRNCPY(new_config.Netif_Config_DNS1, dns1, sizeof(new_config.Netif_Config_DNS1));
            BSP_ARR_STREND(new_config.Netif_Config_DNS1);
        }
#endif

        // check new config valid ,then write and commit
        if (Ethernetif_IsConfigValid(obj, &new_config) &&
            Config_Storage_Write(Dev_ConfigStorage, obj, &new_config) &&
            Config_Storage_Commit(Dev_ConfigStorage))
            return Default_Success_Response(file);
        else
            return Default_Fail_Response(file);
    }

    return Act_Fail_Response(file);
}

static WebApi_Result_t Webapi_DHCP_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    char *act = ReadParam(ACT_STR, iNumParams, pcParam, pcValue);
    void *obj = Dev_Ethernetif_Default;
    const Ethernetif_ConfigFile_t *config = (Ethernetif_ConfigFile_t *)
        Config_Storage_Read(Dev_ConfigStorage, obj, NULL);

    if (config == NULL)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_503_UNAVAILABLE);
        return WEBAPI_ERR_FAILED;
    }
    else if (BSP_STRCMP((const char *)act, GET_STR) == 0)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_200_OK);
        HttpBuilder_printf(file, "{\"DHCP\":\"%s\"}", config->Netif_Config_DHCP_Enable ? TRUE_STR : FALSE_STR);
        return WEBAPI_OK;
    }
    else if (BSP_STRCMP((const char *)act, SET_STR) == 0)
    {
        Ethernetif_ConfigFile_t new_config = *config; // copy original config
        char *value = ReadParam(VALUE_STR, iNumParams, pcParam, pcValue);

        if (BSP_STRCMP(value, TRUE_STR) == 0)
            new_config.Netif_Config_DHCP_Enable = true;
        else if (BSP_STRCMP(value, FALSE_STR) == 0)
            new_config.Netif_Config_DHCP_Enable = false;

        if (config->Netif_Config_DHCP_Enable != new_config.Netif_Config_DHCP_Enable &&
            Ethernetif_IsConfigValid(obj, &new_config) &&
            Config_Storage_Write(Dev_ConfigStorage, obj, &new_config) &&
            Config_Storage_Commit(Dev_ConfigStorage))
            return Default_Success_Response(file);
        else
            return Default_Fail_Response(file);
    }

    return Act_Fail_Response(file);
}

static WebApi_Result_t Webapi_SNTP_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    char *act = ReadParam(ACT_STR, iNumParams, pcParam, pcValue);
    void *obj = App_NetTime;
    const NetTime_ConfigFile_t *config = (NetTime_ConfigFile_t *)
        Config_Storage_Read(Dev_ConfigStorage, obj, NULL);

    if (config == NULL)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_503_UNAVAILABLE);
        return WEBAPI_ERR_FAILED;
    }
    else if (BSP_STRCMP((const char *)act, GET_STR) == 0)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_200_OK);
        HttpBuilder_printf(file, "{\"enable\":\"%s\",", config->NetTime_Enable? TRUE_STR : FALSE_STR);
        HttpBuilder_printf(file, "\"server\":\"%s\"}", config->NetTime_SNTP_Server);
        return WEBAPI_OK;
    }
    else if (BSP_STRCMP((const char *)act, SET_STR) == 0)
    {
        NetTime_ConfigFile_t new_config = *config; //copy original config
        char *enable = ReadParam("enable", iNumParams, pcParam, pcValue);
        char *server = ReadParam("server", iNumParams, pcParam, pcValue);

        if (server && BSP_STRLEN(server))
        {
            BSP_STRNCPY(new_config.NetTime_SNTP_Server, server,
                        sizeof(new_config.NetTime_SNTP_Server));
            BSP_ARR_STREND(new_config.NetTime_SNTP_Server);
        }

        if(enable && BSP_STRCMP(enable, TRUE_STR) == 0)
            new_config.NetTime_Enable = true;
        else if(enable && BSP_STRCMP(enable, FALSE_STR) == 0)
            new_config.NetTime_Enable = false;
        
        if (NetTime_IsConfigValid(obj, &new_config) &&
            Config_Storage_Write(Dev_ConfigStorage, obj, &new_config) &&
            Config_Storage_Commit(Dev_ConfigStorage))
            return Default_Success_Response(file);
        else
            return Default_Fail_Response(file);
    }

    return Act_Fail_Response(file);
}

static WebApi_Result_t Webapi_Current_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    char *act = ReadParam(ACT_STR, iNumParams, pcParam, pcValue);
    void *obj = Dev_CurrentTrig;
    const Device_CurrentTrig_ConfigFile_t *config = (Device_CurrentTrig_ConfigFile_t *)
        Config_Storage_Read(Dev_ConfigStorage, obj, NULL);

    if (config == NULL)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_503_UNAVAILABLE);
        return WEBAPI_ERR_FAILED;
    }
    else if (BSP_STRCMP((const char *)act, GET_STR) == 0)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_200_OK);
        HttpBuilder_printf(file, "{\"disconnect\":%d,", config->CurrentTrig_Disconnect_Thres_mA);
        HttpBuilder_printf(file, "\"overload\":%d,",  config->CurrentTrig_Overload_Thres_mA);
        HttpBuilder_printf(file, "\"threshold\":%d,", config->CurrentTrig_PeakThreshold_1000x);
        HttpBuilder_printf(file, "\"influence\":%d}", config->CurrentTrig_PeakInfluence_1000x);
        return WEBAPI_OK;
    }
    else if (BSP_STRCMP((const char *)act, SET_STR) == 0)
    {
        Device_CurrentTrig_ConfigFile_t new_config = *config; // copy original config
        char *disconnect = ReadParam("disconnect", iNumParams, pcParam, pcValue);
        char *overload = ReadParam("overload", iNumParams, pcParam, pcValue);
        char *threshold = ReadParam("threshold", iNumParams, pcParam, pcValue);
        char *influence = ReadParam("influence", iNumParams, pcParam, pcValue);

        if (disconnect)
            new_config.CurrentTrig_Disconnect_Thres_mA=atoi(disconnect);
        if (overload)
            new_config.CurrentTrig_Overload_Thres_mA=atoi(overload);
        if (threshold)
            new_config.CurrentTrig_PeakThreshold_1000x=atoi(threshold);
        if (influence)
            new_config.CurrentTrig_PeakInfluence_1000x=atoi(influence);

        if (Device_CurrentTrig_IsConfigValid(obj, &new_config) &&
            Config_Storage_Write(Dev_ConfigStorage, obj, &new_config) &&
            Config_Storage_Commit(Dev_ConfigStorage))
            return Default_Success_Response(file);
        else
            return Default_Fail_Response(file);
    }

    return Act_Fail_Response(file);
}

static WebApi_Result_t Webapi_Light_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    char *act = ReadParam(ACT_STR, iNumParams, pcParam, pcValue);
    char *pos = ReadParam("pos", iNumParams, pcParam, pcValue);

    void *obj;
    const Device_FlashLight_ConfigFile_t *config;
    
    if(BSP_STRCMP(pos, "top") == 0)
        obj = Dev_FlashLight_Top;
    else if(BSP_STRCMP(pos, "bottom") == 0)
        obj = Dev_FlashLight_Bottom;
    
    config = (Device_FlashLight_ConfigFile_t *)
        Config_Storage_Read(Dev_ConfigStorage, obj, NULL);
    
    if (config == NULL)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_503_UNAVAILABLE);
        return WEBAPI_ERR_FAILED;
    }
    else if (BSP_STRCMP((const char *)act, GET_STR) == 0)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_200_OK);
        HttpBuilder_printf(file, "{\"%s\":%d}",VALUE_STR, config->FlashLight_Brightness);
        return WEBAPI_OK;
    }
    else if (BSP_STRCMP((const char *)act, SET_STR) == 0)
    {
        Device_FlashLight_ConfigFile_t new_config = *config; // copy original config
        char *value = ReadParam(VALUE_STR, iNumParams, pcParam, pcValue);

        if (value)
        {
            new_config.FlashLight_Brightness = atoi(value);
        }
        if (Device_FlashLight_IsConfigValid(obj, &new_config) &&
            Config_Storage_Write(Dev_ConfigStorage, obj, &new_config) &&
            Config_Storage_Commit(Dev_ConfigStorage))
            return Default_Success_Response(file);
        else
            return Default_Fail_Response(file);
    }

    return Act_Fail_Response(file);
}

static WebApi_Result_t Webapi_Camera_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    char *act = ReadParam(ACT_STR, iNumParams, pcParam, pcValue);
    void *obj = Dev_Cam_OV2640;
    const Device_CamOV2640_ConfigFile_t *config = (Device_CamOV2640_ConfigFile_t*)
        Config_Storage_Read(Dev_ConfigStorage, obj, NULL);

    if (config == NULL)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_503_UNAVAILABLE);
        return WEBAPI_ERR_FAILED;
    }
    else if (BSP_STRCMP((const char *)act, GET_STR) == 0)
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_200_OK);
        HttpBuilder_printf(file, "{\"qs\":%d,", config->CamOV2640_Qs);
        HttpBuilder_printf(file, "\"brightness\":%d,", config->CamOV2640_Brightness);
        HttpBuilder_printf(file, "\"contrast\":%d,", config->CamOV2640_Contrast);
        HttpBuilder_printf(file, "\"flip\":\"%s\",", config->CamOV2640_Flip?TRUE_STR:FALSE_STR);
        HttpBuilder_printf(file, "\"mirror\":\"%s\"}", config->CamOV2640_Mirror?TRUE_STR:FALSE_STR);
        return WEBAPI_OK;
    }
    else if (BSP_STRCMP((const char *)act, SET_STR) == 0)
    {
        Device_CamOV2640_ConfigFile_t new_config = *config; // copy original config
        char *qs = ReadParam("qs", iNumParams, pcParam, pcValue);
        char *brightness = ReadParam("brightness", iNumParams, pcParam, pcValue);
        char *contrast = ReadParam("contrast", iNumParams, pcParam, pcValue);
        char *flip = ReadParam("flip", iNumParams, pcParam, pcValue);
        char *mirror = ReadParam("mirror", iNumParams, pcParam, pcValue);

        if (qs)
            new_config.CamOV2640_Qs = atoi(qs);
        if (brightness)
            new_config.CamOV2640_Brightness = atoi(brightness);
        if (contrast)
            new_config.CamOV2640_Contrast = atoi(contrast);
        if (flip)
        {
            if (BSP_STRCMP(flip, TRUE_STR) == 0)
                new_config.CamOV2640_Flip = true;
            else if (BSP_STRCMP(flip, FALSE_STR) == 0)
                new_config.CamOV2640_Flip = false;
        }
        if (mirror)
        {
            if (BSP_STRCMP(mirror, TRUE_STR) == 0)
                new_config.CamOV2640_Mirror = true;
            else if (BSP_STRCMP(mirror, FALSE_STR) == 0)
                new_config.CamOV2640_Mirror = false;
        }

        if (Device_CamOV2640_IsConfigValid(obj, &new_config) &&
            Config_Storage_Write(Dev_ConfigStorage, obj, &new_config) &&
            Config_Storage_Commit(Dev_ConfigStorage))
            return Default_Success_Response(file);
        else
            return Default_Fail_Response(file);
    }

    return Act_Fail_Response(file);
}

static void __Reboot_Callback(void* arg){ SysCtrl_Reset(); }
static WebApi_Result_t Webapi_Reboot_Handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    char *pw = ReadParam("pw", iNumParams, pcParam, pcValue);
    if (BSP_STRCMP((const char *)pw, REBOOT_PW) == 0)
    {
        DBG_INFO("System reboot in %d second...\n",WEBAPI_REBOOT_DELAY);
        sys_timeout(WEBAPI_REBOOT_DELAY,__Reboot_Callback,NULL);
        return Default_Success_Response(file);
    }
    else
    {
        HttpBuilder_BuildResponse(file, HTTP_RESPONSE_401_UNAUTHORIZED);
        return WEBAPI_ERR_FAILED;
    }
}
#endif
// extern function definition, called by lwip
void httpd_cgi_handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue)
{
    uint16_t i;

    if (file->is_custom_file && BSP_STRCMP(uri, Webapi_Enter_Point) == 0)
    {
        char *cmd = ReadParam(CMD_STR, iNumParams, pcParam, pcValue);

        if (cmd == NULL || strlen(cmd) <= 0)
        {
            HttpBuilder_BuildResponse(file, HTTP_RESPONSE_400_BAD_REQUEST);
        }
        else
        {
            WebApi_Result_t result = WEBAPI_ERR_NOTFOUND;

            for (i = 0; i < BSP_ARR_LEN(Webapi_cmds); i++)
            {
                Webapi_Cmd_FuncPtr_Map_t cmf_func = Webapi_cmds[i];
                if (BSP_STRCMP(cmd, cmf_func.cmd_str) == 0)
                {
                    result = cmf_func.FuncPtr(file, uri, iNumParams, pcParam, pcValue);
                    break;
                }
            }
            // cmd not found
            if (result == WEBAPI_ERR_NOTFOUND)
            {
                HttpBuilder_BuildResponse(file, HTTP_RESPONSE_501_NOT_IMPLEMENTED);
            }
        }
        DBG_INFO("api request cmd:%s\r\n", cmd);
        HttpBuilder_FinishFile(file);
    }
    else
    {
        /* There are 2 conditions we'll get here:
        1. File uri not match Webapi_Enter_Point
        2. File uri match Webapi_Enter_Point, but file->is_custom_file is false 
           caused my mem_malloc fail in fs_open_custom.
        
        Either way, mem_malloc is not success,
        we'll just open and return 404.html to client.
        DO NOT use fs_open to return dynamic webapi content other place,
        it will make alloced mem not free and memory leak.  */
        fs_open(file,"/404.html");
    }
}
