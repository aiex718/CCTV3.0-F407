#include "app/httpd/webapi.h"
#include "app/httpd/http_builder.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/dbg_serial.h"
#include "lwip/ip4_addr.h"
#include "stdio.h"
#include "string.h"

const char WEBAPI_ENTRY_POINT[]="/api";

static const cmd_FuncPtr_Map_t Webapi_cmds[]=
{
    {"uptime",httpd_api_uptime},
};
const uint8_t Webapi_cmds_len = sizeof(Webapi_cmds) / sizeof(cmd_FuncPtr_Map_t);

const char CMD_STR[]="cmd", GET_STR[]="get", SET_STR[]="set";
const char BAD_ARGUMENT_MESSAGE[] = "Bad args.";
const char VALUE_STR[]="value",TRUE_STR[]="true",FALSE_STR[]="false";
const char RESULT_TRUE_JSON[]="{\"result\":\"true\"}";
const char RESULT_FALSE_JSON[]="{\"result\":\"false\"}";

static char* ReadParam(const char *name,int iNumParams,char **pcParam, char **pcValue)
{
    while (--iNumParams>=0)
    {
        if (strcmp(pcParam[iNumParams] , name)==0)   
            return pcValue[iNumParams];     
    }
    return NULL;
}

WebApi_Result_t httpd_api_uptime(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue)
{
    HttpBuilder_BuildResponse(file,HTTP_RESPONSE_200_OK);
    HttpBuilder_printf(file,"{\"uptime\":%u}",sys_now());
    //HttpBuilder_printf(file,"\"nowtime\":%u}",nettime_rtc_GetNowTime());
    return WEBAPI_OK;
}


// extern function definition, called by lwip
void httpd_cgi_handler(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue)
{
    uint16_t i;

    if(file->is_custom_file && strcmp(uri,WEBAPI_ENTRY_POINT)==0)
    {   
        // if(httpd_api_login(iNumParams,pcParam,pcValue)==0)
        // {
        //     HttpBuilder_BuildResponse(file,HTTP_RESPONSE_401_UNAUTHORIZED);
        // }
        // else
        {
            char* cmd=ReadParam(CMD_STR,iNumParams,pcParam,pcValue);
            
            if(cmd==NULL || strlen(cmd)<=0)
            {
                HttpBuilder_BuildResponse(file,HTTP_RESPONSE_400_BAD_REQUEST);
            }
            else
            {
                for (i = 0; i < Webapi_cmds_len; i++)
                {
                    cmd_FuncPtr_Map_t cmf_func = Webapi_cmds[i];
                    if(strcmp(cmd , cmf_func.cmd_str)==0)
                    {
                        cmf_func.FuncPtr(file,uri,iNumParams,pcParam,pcValue);
                        i = Webapi_cmds_len;
                        break;
                    }
                }
                //cmd not found
                if(i != Webapi_cmds_len)
                    HttpBuilder_BuildResponse(file,HTTP_RESPONSE_501_NOT_IMPLEMENTED);
            }
            DBG_INFO("api request cmd:%s\r\n",cmd);
        }
        HttpBuilder_FinishFile(file);
    }
    else
    {
        fs_open(file,"/404.html");
    }
}
