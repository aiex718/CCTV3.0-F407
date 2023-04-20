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

//content-len= total - 101
static const char Http_ViewFps_Response[]= "\
HTTP/1.1 200 OK\r\n\
Content-Length: 6548\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Connection: close\r\n\
\r\n\
<html>\r\n\
    <!-- Ref: https://github.com/aruntj/mjpeg-readable-stream -->\r\n\
    <img id=\"image\" />\r\n\
    <br>\r\n\
    <a>FPS: </a><a id=\"fps\"></a>\r\n\
    <br>\r\n\
    <a>FrameTime UTC: </a><a id=\"frametimeutc\"></a>\r\n\
    <br>\r\n\
    <a>FrameTime Local: </a><a id=\"frametimelocal\"></a>\r\n\
    <br>\r\n\
    <a>FrameTick: </a><a id=\"frametick\"></a>\r\n\
    <br>\r\n\
\r\n\
    <script>\r\n\
        const url = '/stream';\r\n\
\r\n\
        const JPEG_SOI = new Uint8Array(2);\r\n\
        JPEG_SOI[0] = 0xFF;\r\n\
        JPEG_SOI[1] = 0xD8;\r\n\
        const JPEG_COM = new Uint8Array(2);\r\n\
        JPEG_COM[0] = 0xFF;\r\n\
        JPEG_COM[1] = 0xFE;\r\n\
\r\n\
        const CONTENT_LENGTH = 'content-length';\r\n\
        const TYPE_JPEG = 'image/jpeg';\r\n\
        let image = document.getElementById('image');\r\n\
        let fps = document.getElementById('fps');\r\n\
        let frametimeutc = document.getElementById('frametimeutc');\r\n\
        let frametimelocal = document.getElementById('frametimelocal');\r\n\
        let frametick = document.getElementById('frametick');\r\n\
\r\n\
        fetch(url)\r\n\
        .then(response => {\r\n\
            if (!response.ok) {\r\n\
                throw Error(response.status+' '+response.statusText)\r\n\
            }\r\n\
            if (!response.body) {\r\n\
                alert('ReadableStream not supported in this browser.')\r\n\
            }\r\n\
\r\n\
            const reader = response.body.getReader();\r\n\
            let headers = '';\r\n\
            let contentLength = -1;\r\n\
            let imageBuffer = null;\r\n\
            let bytesRead = 0;\r\n\
            \r\n\
            let frames = 0;\r\n\
            \r\n\
            setInterval(() => {\r\n\
                console.log('fps : ' + frames);\r\n\
                fps.innerHTML = frames;\r\n\
                frames = 0;\r\n\
            }, 1000) \r\n\
\r\n\
\r\n\
            const read = () => {\r\n\
\r\n\
                reader.read().then(({done, value}) => {\r\n\
                    if (done) {\r\n\
                        controller.close();\r\n\
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
                                    if(comment_len<=14)\r\n\
                                    {\r\n\
                                        console.log('comment_len error');\r\n\
                                        break;\r\n\
                                    }\r\n\
\r\n\
                                    //extract 8 byte rtc\r\n\
                                    const rtc_len = 8;\r\n\
                                    let rtc_raw = new Uint8Array(new ArrayBuffer(rtc_len));\r\n\
                                    for (let i = 0; i < rtc_len; i++)\r\n\
                                        rtc_raw[i] = imageBuffer[com_idx+4+i];\r\n\
                                    let WeekDay = rtc_raw[0]; \r\n\
                                    let Month = rtc_raw[1];\r\n\
                                    let Day = rtc_raw[2];\r\n\
                                    let Year = rtc_raw[3];\r\n\
                                    let Hour = rtc_raw[4];\r\n\
                                    let Minute = rtc_raw[5];\r\n\
                                    let Second = rtc_raw[6];\r\n\
                                    let H12 = rtc_raw[7];\r\n\
\r\n\
                                    let datetime = new Date(Year, Month, Day, Hour, Minute, Second);\r\n\
                                    frametimeutc.innerHTML = datetime.toUTCString();\r\n\
                                    let datetime_local = new Date(Year, Month, Day, Hour, Minute, Second);\r\n\
                                    frametimelocal.innerHTML = datetime_local.toLocaleString();\r\n\
\r\n\
                                    //extract 4 byte frame tick\r\n\
                                    const frametick_len = 4;\r\n\
                                    let tick_raw = new Uint8Array(new ArrayBuffer(frametick_len));\r\n\
                                    for (let i = 0; i < frametick_len; i++)\r\n\
                                        tick_raw[i] = imageBuffer[com_idx+rtc_len+4+i];\r\n\
                                    let tick = 0;\r\n\
                                    for (let i = 0; i < frametick_len; i++)\r\n\
                                        tick += tick_raw[i] << (8 * i);\r\n\
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
                            frames++;\r\n\
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
    </script>\r\n\
</html>";
