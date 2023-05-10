#ifndef WEBAPI_H
#define WEBAPI_H

#include "bsp/platform/platform_defs.h"

#include "lwip/apps/fs.h"//struct fs_file

#ifndef WEBAPI_RESPONSE_BUFFER_LEN
    #define WEBAPI_RESPONSE_BUFFER_LEN 256
#endif

extern const char* const Webapi_Enter_Point;

typedef enum {
    WEBAPI_OK = 0,
    WEBAPI_ERR_NOTFOUND,
} WebApi_Result_t;

WebApi_Result_t httpd_api_uptime(struct fs_file *file, \
    const char* uri, int iNumParams,char **pcParam, char **pcValue);

#endif


