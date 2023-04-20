#ifndef __MJPEGD_MACRO_H__
#define __MJPEGD_MACRO_H__

#define MJPEGD_MIN(a,b) (((a)<(b))?(a):(b))
#define MJPEGD_MAX(a,b) (((a)>(b))?(a):(b))
#define MJPEGD_ARRLEN(s) (sizeof(s)/sizeof(s[0]))
#define MJPEGD_STRLEN(s) (sizeof(s)/sizeof(s[0])-1)
#define MJPEGD_MEMCPY MEMCPY
#define MJPEGD_STRNCMP strncmp

__STATIC_INLINE char* MJPEGD_STRNSTR(const char* buffer, const char* token, size_t n)
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
