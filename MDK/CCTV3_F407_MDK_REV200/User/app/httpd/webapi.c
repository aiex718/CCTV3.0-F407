#include "app/httpd/webapi.h"
#include "app/httpd/http_builder.h"
#include "bsp/sys/dbg_serial.h"

#include "lwip/apps/fs.h"
#include "lwip/ip4_addr.h"

const char* const Webapi_Enter_Point="/api";

typedef enum {
    WEBAPI_OK = 0,
    WEBAPI_ERR_NOTFOUND,
} WebApi_Result_t;

typedef struct 
{
    char* cmd_str;
    WebApi_Result_t (*FuncPtr)(struct fs_file *file, \
        const char* uri, int iNumParams,char **pcParam, char **pcValue);
}Webapi_Cmd_FuncPtr_Map_t;

static WebApi_Result_t Webapi_Get_Uptime(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue);

static const Webapi_Cmd_FuncPtr_Map_t Webapi_cmds[]=
{
    {"uptime",Webapi_Get_Uptime},
};

const char CMD_STR[]="cmd", GET_STR[]="get", SET_STR[]="set";
const char BAD_ARGUMENT_MESSAGE[] = "Bad args.";
const char VALUE_STR[]="value",TRUE_STR[]="true",FALSE_STR[]="false";
const char RESULT_TRUE_JSON[]="{\"result\":\"true\"}";
const char RESULT_FALSE_JSON[]="{\"result\":\"false\"}";

__STATIC_INLINE char* ReadParam(const char *name,int iNumParams,char **pcParam, char **pcValue)
{
    while (--iNumParams>=0)
    {
        if (BSP_STRCMP(pcParam[iNumParams] , name)==0)   
            return pcValue[iNumParams];     
    }
    return NULL;
}

static WebApi_Result_t Webapi_Get_Uptime(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue)
{
    HttpBuilder_BuildResponse(file,HTTP_RESPONSE_200_OK);
    HttpBuilder_printf(file,"{\"uptime\":%u,",sys_now());
    HttpBuilder_printf(file,"\"nowtime\":%u}",0);
    return WEBAPI_OK;
}


// extern function definition, called by lwip
void httpd_cgi_handler(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue)
{
    uint16_t i;

    if(file->is_custom_file && strcmp(uri,Webapi_Enter_Point)==0)
    {   
        char* cmd=ReadParam(CMD_STR,iNumParams,pcParam,pcValue);
            
        if(cmd==NULL || strlen(cmd)<=0)
        {
            HttpBuilder_BuildResponse(file,HTTP_RESPONSE_400_BAD_REQUEST);
        }
        else
        {
            WebApi_Result_t result = WEBAPI_ERR_NOTFOUND;
            for (i = 0; i < BSP_ARR_LEN(Webapi_cmds); i++)
            {
                Webapi_Cmd_FuncPtr_Map_t cmf_func = Webapi_cmds[i];
                if(strcmp(cmd , cmf_func.cmd_str)==0)
                {
                    result = cmf_func.FuncPtr(file,uri,iNumParams,pcParam,pcValue);
                    break;
                }
            }
            //cmd not found
            if(result == WEBAPI_ERR_NOTFOUND)
                HttpBuilder_BuildResponse(file,HTTP_RESPONSE_501_NOT_IMPLEMENTED);
        }
        DBG_INFO("api request cmd:%s\r\n",cmd);
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
