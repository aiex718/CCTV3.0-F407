#ifndef __MJPEGD_MACRO_H__
#define __MJPEGD_MACRO_H__

#include "bsp/sys/atomic.h"
#include "stdlib.h"
#include "string.h"

#define MJPEGD_MIN(a,b) (((a)<(b))?(a):(b))
#define MJPEGD_MAX(a,b) (((a)>(b))?(a):(b))
#define MJPEGD_ARRLEN(s) (sizeof(s)/sizeof(s[0]))
#define MJPEGD_CHRARR_STRLEN(s) (MJPEGD_ARRLEN(s)-1)
#define MJPEGD_MEMCPY MEMCPY
#define MJPEGD_STRNCMP strncmp
#define MJPEGD_STRNSTR strnstr
#define MJPEGD_STRCHR strchr
#define MJPEGD_MEMSET memset
#define MJPEGD_ATOI atoi

#define MJPEGD_ATOMIC_XCHG(p,val) Atomic_Exchange((p),(val))
#define MJPEGD_ATOMIC_INC(p) Atomic_Increment((p))


static char* strnstr(const char* buffer, const char* token, size_t n)
{
    const char* p;
    size_t tokenlen = strlen(token);
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
