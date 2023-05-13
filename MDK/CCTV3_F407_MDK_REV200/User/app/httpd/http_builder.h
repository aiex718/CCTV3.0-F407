#ifndef HTTP_BUILDER_H
#define HTTP_BUILDER_H

#include "lwip/apps/fs.h"

typedef enum {
    HTTP_RESPONSE_200_OK = 0,
    HTTP_RESPONSE_400_BAD_REQUEST,
    HTTP_RESPONSE_401_UNAUTHORIZED,
    HTTP_RESPONSE_404_NOTFOUNT,
    HTTP_RESPONSE_501_NOT_IMPLEMENTED,
    HTTP_RESPONSE_503_UNAVAILABLE,
    __NOT_RESPONSE_CODE_MAX,
} HttpBuilder_ResponseCode_t;

typedef enum {
    HTTP_BUILDER_OK = 0,
    HTTP_BUILDER_ERR_ARG,
    HTTP_BUILDER_ERR_TOO_MANY,
    HTTP_BUILDER_BAD_CONTENT,
} HttpBuilder_Status_t;

/**
 * @brief Insert content to fsfile using printf format
 * @warning This function wont check overflow, make sure the fsfile is large enough
 * @return CGI_HANDLER_OK if success, other if error
 */
HttpBuilder_Status_t HttpBuilder_printf(struct fs_file *file,const char* str,...);
HttpBuilder_Status_t HttpBuilder_Insert(struct fs_file *file,const char* str,uint16_t max_len);
HttpBuilder_Status_t HttpBuilder_BuildResponse(struct fs_file *file,HttpBuilder_ResponseCode_t code);
HttpBuilder_Status_t HttpBuilder_FinishFile(struct fs_file *file);

#endif
