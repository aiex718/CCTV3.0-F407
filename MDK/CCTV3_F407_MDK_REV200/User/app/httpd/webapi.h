#ifndef WEBAPI_H
#define WEBAPI_H

#include "lwip/apps/fs.h"//struct fs_file

#define WEBAPI_RESPONSE_BUFFER_LEN 256
extern const char WEBAPI_ENTRY_POINT[];

typedef enum {
    WEBAPI_OK = 0,
    WEBAPI_ERR_,
} WebApi_Result_t;

typedef struct 
{
    char* cmd_str;
    WebApi_Result_t (*FuncPtr)\
        (struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue);
} cmd_FuncPtr_Map_t;

extern const cmd_FuncPtr_Map_t Webapi_cmds[];
extern const uint8_t  Webapi_cmds_len;

// WebApi_Result_t httpd_api_login(int iNumParams,char **pcParam, char **pcValue);
// WebApi_Result_t httpd_api_ip(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue);
// WebApi_Result_t httpd_api_dhcp(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue);
// WebApi_Result_t httpd_api_reboot(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue);
WebApi_Result_t httpd_api_uptime(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue);
//WebApi_Result_t httpd_api_formatdisk(struct fs_file *file, const char* uri, int iNumParams,char **pcParam, char **pcValue);

#endif


