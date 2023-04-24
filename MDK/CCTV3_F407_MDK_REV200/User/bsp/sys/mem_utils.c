#include "bsp/sys/mem_utils.h"

void* mem_search
    (const uint8_t *data, uint16_t data_len, 
     const uint8_t *search, uint16_t search_len, bool from_end)
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
