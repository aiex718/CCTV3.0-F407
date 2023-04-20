#ifndef __MJPEGD_MACRO_H__
#define __MJPEGD_MACRO_H__

#define MJPEGD_MIN(a,b) (((a)<(b))?(a):(b))
#define MJPEGD_MAX(a,b) (((a)>(b))?(a):(b))
#define MJPEGD_ARRLEN(s) (sizeof(s)/sizeof(s[0]))
#define MJPEGD_STRLEN(s) (sizeof(s)/sizeof(s[0])-1)
#define MJPEGD_MEMCPY MEMCPY
#define MJPEGD_STRNCMP strncmp

static char* MJPEGD_STRNSTR(const char* buffer, const char* token, size_t n)
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

static void* MJPEGD_MEMSEARCH
    (const u8_t *data, u16_t data_len, 
     const u8_t *search, u16_t search_len, u8_t from_end)
{
    int8_t inc=from_end?-1:1;
    if(data_len==0 || search_len==0 || data_len<search_len)
        return NULL;
    if(from_end)
        data+=data_len - search_len;
    data_len=data_len - search_len + 1;
    while(data_len--)
    {
        if(BSP_MEMCMP(data,search,search_len)==0)
            return (void*)data;

        data+=inc;
    }
    return NULL;
}

#endif // __MJPEGD_MACRO_H__
