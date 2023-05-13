#include "app/httpd/http_builder.h"
#include "app/httpd/webapi.h"

#include "lwip/mem.h"

#include "string.h"
#include "stdio.h"
#include "stdarg.h"

static const char* const http_response_header[__NOT_RESPONSE_CODE_MAX]=
{
    (const char*)"HTTP/1.0 200 OK \r\n",
    (const char*)"HTTP/1.0 400 Bad Request\r\n",
    (const char*)"HTTP/1.0 401 Unauthorized\r\n",
    (const char*)"HTTP/1.0 404 Not Found\r\n",
    (const char*)"HTTP/1.0 501 Not Implemented\r\n",
    (const char*)"HTTP/1.0 503 Service Unavailable\r\n",
};

static const char* const http_response_default_msg[__NOT_RESPONSE_CODE_MAX]=
{
    NULL,
    NULL,
    (const char*)"Api Unauthorized user",
    (const char*)"Api not found",
    (const char*)"Api command not implemented",
    (const char*)"Api service unavailable",
};

static const char header_http_server[]="Server: lwIP 2.1.3(Alex.C)\r\n";
static const char header_http_content_type_json[]="Content-type: application/json\r\n";
//static const char header_http_content_type_html[]="Content-type: text/html\r\n";
static const char header_http_content_length[]="Content-Length: 0     \r\n";
static const char header_http_closed[]="Connection: Closed\r\n";
static const char http_end_clrf[]="\r\n";

static const char http_header_end[]="\r\n\r\n";

//Private function declaration
static HttpBuilder_Status_t HttpBuilder_ModifyContentLength(struct fs_file *file)
{
    char buf[6]={0};
    char* str_find;
    uint32_t content_len, overwrite_len;
    
    HttpBuilder_Insert(file,http_end_clrf,0);

    //calculate content length
    str_find = strstr(file->data,http_header_end);
    if(str_find==NULL) return HTTP_BUILDER_BAD_CONTENT; 
    content_len = file->len - ((uint32_t)str_find+4 - (uint32_t)file->data);

    str_find = strstr(file->data,header_http_content_length);
    if(str_find==NULL) return HTTP_BUILDER_BAD_CONTENT; 

    str_find+=16;//offset to numbers        
    overwrite_len = sprintf(buf,"%d",content_len);//len should be less then 4 digits
    strncpy(str_find,buf,overwrite_len);
    
    //DBG_INFO("after file->data:%s\r\n",file->data);
    return HTTP_BUILDER_OK;
}

HttpBuilder_Status_t HttpBuilder_printf(struct fs_file *file,const char* str,...)
{
    uint16_t len;    
    if(file==NULL) return HTTP_BUILDER_ERR_ARG;

    va_list argptr;
    va_start(argptr, str);
    len = vsprintf((char*)(file->data)+file->len,str,argptr);    
    file->len += len;
    va_end(argptr);

    return HTTP_BUILDER_OK;
}

HttpBuilder_Status_t HttpBuilder_Insert(struct fs_file *file,const char* str,uint16_t max_len)
{
    uint16_t len=strlen(str);
    if(max_len && len>max_len)
        len = max_len;

    if(file==NULL || str == NULL || len<=0) return HTTP_BUILDER_ERR_ARG;
    if(len>(WEBAPI_RESPONSE_BUFFER_LEN-file->len)) return HTTP_BUILDER_ERR_TOO_MANY;

    MEMCPY((char*)(file->data)+file->len,str,len);
    file->len += len;

    return HTTP_BUILDER_OK;
}

HttpBuilder_Status_t HttpBuilder_BuildResponse(struct fs_file *file,HttpBuilder_ResponseCode_t code)
{
    if(file==NULL) return HTTP_BUILDER_ERR_ARG;        
    if(code>=__NOT_RESPONSE_CODE_MAX) return HTTP_BUILDER_ERR_ARG;

    HttpBuilder_Insert(file,http_response_header[code],0);//insert header
    HttpBuilder_Insert(file,header_http_server,0);
    HttpBuilder_Insert(file,header_http_content_type_json,0);
    HttpBuilder_Insert(file,header_http_content_length,0);
    HttpBuilder_Insert(file,header_http_closed,0);
    HttpBuilder_Insert(file,http_end_clrf,0);
    HttpBuilder_Insert(file,http_response_default_msg[code],0);//insert default content
    return HTTP_BUILDER_OK;
}

HttpBuilder_Status_t HttpBuilder_FinishFile(struct fs_file *file)
{
    if(file==NULL) return HTTP_BUILDER_ERR_ARG;
    return HttpBuilder_ModifyContentLength(file);
}


