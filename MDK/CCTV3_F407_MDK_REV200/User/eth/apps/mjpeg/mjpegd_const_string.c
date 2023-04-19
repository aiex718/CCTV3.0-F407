/**DO NOT COMPILE THIS FILE, SHOULD BE INCLUDE IN MJPEGD.C**/
//TODO: make a .h
static const char Http_Notfound_Response[]= "\
HTTP/1.1 404 Not Found\r\n\
Content-Type: text/html\r\n\
Content-Length: 39\r\n\
Connection: close  \r\n\
\r\n\
<html>\r\n\
<H1>404 Not Found</H1>\r\n\
</html>";

static const char HTTP_TooMany_Response[]="\
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

static const char Http_Handshake_Response[]= "\
HTTP/1.1 200 No Content\r\n\
Content-Length: 26\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Connection: close \r\n\
\r\n\
<H1>Hello from mjpegd</H1>";

static const char Http_ViewSnap_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Length: 19\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Connection: close  \r\n\
\r\n\
<img src=\"/snap\" />";

static const char Http_ViewStream_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Length: 21\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Connection: close\r\n\
\r\n\
<img src=\"/stream\" />";

static const char Http_Snap_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Type: image/jpeg\r\n\
Transfer-Encoding: chunked\r\n\
Cache-Control: no-cache\r\n\
Connection: close  \r\n\
\r\n\
";

static const char Http_Stream_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Type: multipart/x-mixed-replace;boundary=myboundary\r\n\
Transfer-Encoding: chunked\r\n\
Cache-Control: no-cache\r\n\
Connection: Keep-Alive\r\n\
Keep-Alive: timeout=10   \r\n\
\r\n\
";

static const char Http_Mjpeg_Boundary[]="--myboundary";
static const char Http_Mjpeg_ContentType[]="Content-Type: image/jpg \r\n";
static const char Http_Mjpeg_ContentLength[]="Content-Length: %5d   \r\n\r\n";

static const char Http_CLRF[]="\r\n";

const uint16_t Mjpeg_Jpeg_Comment_wlen = 20;
const uint8_t Mjpeg_Jpeg_Comment[Mjpeg_Jpeg_Comment_wlen+6] =
{
    0xFF,0xD8,0xFF,0xFE,0x00,Mjpeg_Jpeg_Comment_wlen+2,
    //0,0,0,0,0,0,0,0,
    //0,0,0,0,0,0,0,0,
    //0,0,0,0
};
const uint16_t Mjpeg_Jpeg_Comment_len = sizeof(Mjpeg_Jpeg_Comment);

