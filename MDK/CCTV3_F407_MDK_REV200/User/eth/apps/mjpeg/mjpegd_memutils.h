#ifndef __MJPEGD_MACRO_H__
#define __MJPEGD_MACRO_H__

#define MJPEGD_MIN(a,b) (((a)<(b))?(a):(b))
#define MJPEGD_MAX(a,b) (((a)>(b))?(a):(b))
#define MJPEGD_ARRLEN(s) (sizeof(s)/sizeof(s[0]))
#define MJPEGD_CHRARR_STRLEN(s) (MJPEGD_ARRLEN(s)-1)
#define MJPEGD_MEMCPY MEMCPY
#define MJPEGD_STRNCMP strncmp
#define MJPEGD_STRNSTR strnstr

static char* strnstr(const char* buffer, const char* token, size_t n)
{
    const char* p;
    int tokenlen = (int)strlen(token);
    if (tokenlen == 0) {
        return (char *)buffer;
    }
    for (p = buffer; *p && (p + tokenlen <= buffer + n); p++) {
        if ((*p == *token) && (MJPEGD_STRNCMP(p, token, tokenlen) == 0)) {
            return (char *)p;
        }
    }
    return 0;
} 


#endif // __MJPEGD_MACRO_H__
