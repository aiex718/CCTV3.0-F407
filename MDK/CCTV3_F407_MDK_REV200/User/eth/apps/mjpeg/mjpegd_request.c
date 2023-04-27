#include "eth/apps/mjpeg/mjpegd_request.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"
#include "eth/apps/mjpeg/mjpegd_stream.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/mjpegd_opts.h"
#include "eth/apps/mjpeg/trycatch.h"

#if 1 /* Http response strings */
const char Http_Notfound_Response[]= "\
HTTP/1.1 404 Not Found\r\n\
Content-Type: text/html\r\n\
Content-Length: 39\r\n\
Connection: close  \r\n\
\r\n\
<html>\r\n\
<H1>404 Not Found</H1>\r\n\
</html>";

const char HTTP_TooMany_Response[]="\
HTTP/1.1 429 Too Many Requests\r\n\
Connection: close\r\n\
Content-Length: 71\r\n\
Content-Type: text/html\r\n\
Retry-After: 60\r\n\
\r\n\
<html>\r\n\
<H1>429 Too Many Connections</H1>\r\n\
<H4>Retry later<H4>\r\n\
</html>";

const char Http_Handshake_Response[]= "\
HTTP/1.1 200 No Content\r\n\
Content-Length: 26\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Connection: close \r\n\
\r\n\
<H1>Hello from mjpegd</H1>";

const char Http_ViewSnap_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Length: 19\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Connection: close  \r\n\
\r\n\
<img src=\"/snap\" />";

const char Http_ViewStream_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Length: 21\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Connection: close\r\n\
\r\n\
<img src=\"/stream\" />";

#if MJPEGD_ALLOW_STREAM_CORS
const char Http_MjpegChunked_Response[]= "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Type: multipart/x-mixed-replace;boundary=myboundary\r\n\
Transfer-Encoding: chunked\r\n\
Cache-Control: no-cache\r\n\
Connection: Keep-Alive\r\n\
Keep-Alive: timeout=10   \r\n\
\r\n\
";
#else
const char Http_MjpegChunked_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Type: multipart/x-mixed-replace;boundary=myboundary\r\n\
Transfer-Encoding: chunked\r\n\
Cache-Control: no-cache\r\n\
Connection: Keep-Alive\r\n\
Keep-Alive: timeout=10   \r\n\
\r\n\
";
#endif

//content-len= total - 101
const char Http_ViewFps_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Length: 5687\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Connection: close\r\n\
\r\n\
<html>\r\n\
    <!-- Ref: https://github.com/aruntj/mjpeg-readable-stream -->\r\n\
    <img id='image' />\r\n\
    <br>\r\n\
    <a>FPS: </a><a id='fps'>0</a>\r\n\
    <br>\r\n\
    <a>FPS Limit: </a>\r\n\
    <select id='fpslimit'>\r\n\
        <option value='0'>0</option>\r\n\
        <option value='5'>5</option>\r\n\
        <option value='10'>10</option>\r\n\
        <option value='15'>15</option>\r\n\
        <option value='20'>20</option>\r\n\
        <option value='25'>25</option>\r\n\
        <option value='30'>30</option>\r\n\
    </select>\r\n\
    <br>\r\n\
    <a>FrameTime UTC: </a><a id='frametimeutc'></a>\r\n\
    <br>\r\n\
    <a>FrameTime Local: </a><a id='frametimelocal'></a>\r\n\
    <br>\r\n\
    <a>FrameTick: </a><a id='frametick'></a>\r\n\
    <br>\r\n\
\r\n\
    <script>\r\n\
const url = '/stream';\r\n\
const JPEG_SOI = new Uint8Array(2);\r\n\
JPEG_SOI[0] = 0xFF;\r\n\
JPEG_SOI[1] = 0xD8;\r\n\
const JPEG_COM = new Uint8Array(2);\r\n\
JPEG_COM[0] = 0xFF;\r\n\
JPEG_COM[1] = 0xFE;\r\n\
\r\n\
const CONTENT_LENGTH = 'content-length';\r\n\
let image = document.getElementById('image');\r\n\
let fps = document.getElementById('fps');\r\n\
let frametimeutc = document.getElementById('frametimeutc');\r\n\
let frametimelocal = document.getElementById('frametimelocal');\r\n\
let frametick = document.getElementById('frametick');\r\n\
let fpslimit = document.getElementById('fpslimit');\r\n\
let frames_cnt = 0;\r\n\
    \r\n\
setInterval(() => {\r\n\
    console.log('fps : ' + frames_cnt);\r\n\
    fps.innerHTML = frames_cnt;\r\n\
    frames_cnt = 0;\r\n\
}, 1000) \r\n\
\r\n\
fpslimit.onchange = function() {\r\n\
    //restart fetch\r\n\
    if(reader!=null)\r\n\
        reader.cancel();\r\n\
    GetStream(url+'?fps='+fpslimit.value);\r\n\
}\r\n\
\r\n\
let reader = null;\r\n\
function GetStream(url)\r\n\
{\r\n\
    fetch(url)\r\n\
    .then(response => {\r\n\
        if (!response.ok) {\r\n\
            throw Error(response.status+' '+response.statusText)\r\n\
        }\r\n\
        if (!response.body) {\r\n\
            alert('ReadableStream not supported in this browser.')\r\n\
        }\r\n\
\r\n\
        reader = response.body.getReader();\r\n\
        let headers = '';\r\n\
        let contentLength = -1;\r\n\
        let imageBuffer = null;\r\n\
        let bytesRead = 0;\r\n\
\r\n\
        const read = () => {\r\n\
            reader.read().then(({done, value}) => {\r\n\
                if (done) {\r\n\
                    //controller.close();\r\n\
                    return;\r\n\
                }\r\n\
\r\n\
                for (let index =0; index < value.length; index++) {\r\n\
                    if (value[index] === JPEG_SOI[0] && value[index+1] === JPEG_SOI[1]) {\r\n\
                        contentLength = getLength(headers);\r\n\
                        imageBuffer = new Uint8Array(new ArrayBuffer(contentLength));\r\n\
                    }\r\n\
                    if (contentLength <= 0) {\r\n\
                        headers += String.fromCharCode(value[index]);\r\n\
                    }\r\n\
                    else if (bytesRead < contentLength){\r\n\
                        imageBuffer[bytesRead++] = value[index];\r\n\
                    }\r\n\
                    else {\r\n\
                        for (let com_idx=0; com_idx < imageBuffer.length-1; com_idx++)\r\n\
                        {\r\n\
                            if (imageBuffer[com_idx] === JPEG_COM[0] && imageBuffer[com_idx+1] === JPEG_COM[1])\r\n\
                            {\r\n\
                                let comment_len = imageBuffer[com_idx+2] * 256 + imageBuffer[com_idx+3];\r\n\
                                if(comment_len<8)\r\n\
                                {\r\n\
                                    console.log('comment_len error');\r\n\
                                    break;\r\n\
                                }\r\n\
\r\n\
                                let r_idx = com_idx+4;\r\n\
\r\n\
                                //extract 4 byte timestamp\r\n\
                                const timestamp_len =4;\r\n\
                                let timestamp = 0;\r\n\
                                for (let i = 0; i < timestamp_len; i++)\r\n\
                                    timestamp += imageBuffer[r_idx++] << (8 * i);\r\n\
\r\n\
                                let datetime = new Date(timestamp*1000);\r\n\
                                frametimeutc.innerHTML = datetime.toUTCString();\r\n\
                                frametimelocal.innerHTML = datetime.toLocaleString();\r\n\
\r\n\
                                //extract 4 byte frame tick\r\n\
                                const frametick_len = 4;\r\n\
                                let tick=0;\r\n\
                                for (let i = 0; i < frametick_len; i++)\r\n\
                                    tick += imageBuffer[r_idx++] << (8 * i);\r\n\
                                frametick.innerHTML = tick;\r\n\
\r\n\
                                break;\r\n\
                            }\r\n\
                        }\r\n\
                        \r\n\
                        let frame = URL.createObjectURL(new Blob([imageBuffer])) \r\n\
                        image.src = frame;\r\n\
                        window.setTimeout(function () {\r\n\
                            URL.revokeObjectURL(frame)\r\n\
                        }, 1000);\r\n\
                        \r\n\
                        frames_cnt++;\r\n\
                        contentLength = 0;\r\n\
                        bytesRead = 0;\r\n\
                        headers = '';\r\n\
                    }\r\n\
                }\r\n\
\r\n\
                read();\r\n\
            }).catch(error => {\r\n\
                console.error(error);\r\n\
            })\r\n\
        }\r\n\
        read();\r\n\
    }).catch(error => {\r\n\
        console.error(error);\r\n\
    })\r\n\
}\r\n\
\r\n\
const getLength = (headers) => {\r\n\
    let contentLength = -1;\r\n\
    headers.split('\\n').forEach((header, _) => {\r\n\
        const pair = header.split(':');\r\n\
        if (pair[0].toLowerCase() === CONTENT_LENGTH) { \r\n\
            contentLength = pair[1];\r\n\
        }\r\n\
    })\r\n\
    return contentLength;\r\n\
};\r\n\
\r\n\
GetStream(url);\r\n\
    </script>\r\n\
</html>";
#endif

/** http request handlers **/
const Mjpegd_RequestHandler_t mjpegd_request_handlers[__NOT_REQUEST_MAX]=
{
    {REQUEST_NOTFOUND   ,"/404"         ,Http_Notfound_Response     ,MJPEGD_CHRARR_STRLEN(Http_Notfound_Response)       ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_TOOMANY    ,"/429"         ,HTTP_TooMany_Response      ,MJPEGD_CHRARR_STRLEN(HTTP_TooMany_Response)        ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_HANDSHAKE  ,"/handshake"   ,Http_Handshake_Response    ,MJPEGD_CHRARR_STRLEN(Http_Handshake_Response)      ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_VIEW_SNAP  ,"/view/snap"   ,Http_ViewSnap_Response     ,MJPEGD_CHRARR_STRLEN(Http_ViewSnap_Response)       ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_VIEW_STREAM,"/view/stream" ,Http_ViewStream_Response   ,MJPEGD_CHRARR_STRLEN(Http_ViewStream_Response)     ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_VIEW_FPS   ,"/view/fps"    ,Http_ViewFps_Response      ,MJPEGD_CHRARR_STRLEN(Http_ViewFps_Response)        ,NULL                       ,NULL                           ,NULL                       },
    {REQUEST_SNAP       ,"/snap"        ,Http_MjpegChunked_Response ,MJPEGD_CHRARR_STRLEN(Http_MjpegChunked_Response)   ,Mjpegd_Stream_FrameSent    ,NULL                           ,NULL                       },
    {REQUEST_STREAM     ,"/stream"      ,Http_MjpegChunked_Response ,MJPEGD_CHRARR_STRLEN(Http_MjpegChunked_Response)   ,Mjpegd_Stream_FrameSent    ,Mjpegd_Stream_RecvRequest      ,Mjpegd_Stream_CloseRequest },
};

//private functions
static u8_t Mjpegd_Request_ParseParams(char *url_params,
    char **params_out, char **param_vals_out, const u8_t params_max_len);

err_t Mjpegd_Request_Parse(ClientState_t *cs, char *req, u16_t req_len)
{
    err_t err;

    try
    {
        char *line_end;
        const Mjpegd_RequestHandler_t *handler;
        u8_t i;
        throwif(cs == NULL, NULL_CLIENT);

        line_end = MJPEGD_STRNSTR(req, "\r\n", req_len);
        throwif(req_len < MJPEGD_MIN_REQ_LEN || line_end == NULL, REQUEST_NOT_COMPLETE);
        throwif(MJPEGD_STRNCMP(req, "GET ", 4), NOT_HTTP_GET);

        // default request handler is REQUEST_NOTFOUND
        cs->request_handler = &mjpegd_request_handlers[REQUEST_NOTFOUND];

        for (i = 0; i < MJPEGD_ARRLEN(mjpegd_request_handlers); i++)
        {
            handler = &mjpegd_request_handlers[i];
            if (!MJPEGD_STRNCMP(req + 4, handler->url, strlen(handler->url)))
            {
                cs->request_handler = handler;
                break;
            }
        }

        if (cs->request_handler->req != REQUEST_NOTFOUND)
        {
            // we have a valid request, parse url parameters
            char *param_head = (char *)MJPEGD_STRCHR(req, '?');
            if (param_head != NULL)
            {
                char *params[MJPEGD_MAX_URL_PARAMETERS];
                char *params_vals[MJPEGD_MAX_URL_PARAMETERS];
                u8_t i,param_count =
                    Mjpegd_Request_ParseParams(++param_head, params,
                                           params_vals, MJPEGD_MAX_URL_PARAMETERS);
                for (i = 0; i < param_count; i++)
                {
                    //set fps
                    if(strcmp("fps",params[i])==0)
                    {
                        uint8_t fps = (uint8_t)MJPEGD_ATOI(params_vals[i]);
                        if(fps>0)
                        {
                            cs->fps_period = 1000/fps;
                            LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_TRACE,
                                MJPEGD_DBG_ARG("cs %p set fps=%d, period=%d\n", cs, fps,cs->fps_period));
                        }
                    }
                }
            }
        }

        err = ERR_OK;
    }
    catch (NULL_CLIENT)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    MJPEGD_DBG_ARG("ParseRequest NULL_CLIENT\n"));
        err = ERR_ARG;
    }
    catch (REQUEST_NOT_COMPLETE)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                    MJPEGD_DBG_ARG("ParseRequest REQUEST_NOT_COMPLETE:%s\n", req));

        err = ERR_ARG;
    }
    catch (NOT_HTTP_GET)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                    MJPEGD_DBG_ARG("ParseRequest NOT_HTTP_GET:%s\n", req));

        err = ERR_ARG;
    }
    finally
    {
        return err;
    }
}

err_t Mjpegd_Request_BuildResponse(ClientState_t *cs)
{
    err_t err;

    try
    {
        throwif(cs == NULL, NULL_CS);
        throwif(cs->request_handler == NULL, NULL_REQUEST_HANDLER);

        client_assign_file(cs,
                           (u8_t *)cs->request_handler->response,
                           cs->request_handler->response_len);

        cs->get_nextfile_func = cs->request_handler->get_nextfile_func;
        err = ERR_OK;
    }
    catch (NULL_CS)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    MJPEGD_DBG_ARG("response NULL_CLIENT\n"));
        err = ERR_ARG;
    }
    catch (NULL_REQUEST_HANDLER)
    {
        LWIP_DEBUGF(MJPEGD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
                    MJPEGD_DBG_ARG("response NULL_REQUEST_HANDLER\n"));

        err = ERR_ARG;
    }
    finally
    {
        return err;
    }
}

static u8_t Mjpegd_Request_ParseParams(char *url_params,
    char **params_out, char **param_vals_out, const u8_t params_max_len)
{
    char *pair;
    char *equals;
    u8_t loop;

    /* If we have no parameters at all, return immediately. */
    if (!url_params || (*url_params == '\0') || (*url_params == ' '))
        return 0;

    /* Get a pointer to our first parameter */
    pair = url_params;

    /* Parse up to LWIP_HTTPD_MAX_CGI_PARAMETERS from the passed string and ignore the
     * remainder (if any) */
    for (loop = 0; (loop < params_max_len) && pair; loop++)
    {
        /* Save the name of the parameter */
        params_out[loop] = pair;
        /* Remember the start of this name=value pair */
        equals = pair;
        /* Find the start of the next name=value pair and replace the delimiter
         * with a 0 to terminate the previous pair string. */
        pair = MJPEGD_STRCHR(pair, '&');
        if (pair)
        {
            *pair = '\0';
            pair++;
        }
        else
        {
            /* We didn't find a new parameter so find the end of the URI and
             * replace the space with a '\0' */
            pair = MJPEGD_STRCHR(equals, ' ');
            if (pair)
            {
                *pair = '\0';
            }
            /* Revert to NULL so that we exit the loop as expected. */
            pair = NULL;
        }
        /* Now find the '=' in the previous pair, replace it with '\0' and save
         * the parameter value string. */
        equals = MJPEGD_STRCHR(equals, '=');
        if (equals)
        {
            *equals = '\0';
            param_vals_out[loop] = equals + 1;
        }
        else
            param_vals_out[loop] = NULL;
    }
    return loop;
}

