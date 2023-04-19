#ifndef MEM_UTILS_H
#define MEM_UTILS_H

#include "bsp/platform/platform_defs.h"

//TODO: Do a fast memcpy
//TODO: Add to BSP
void* mem_search
    (const uint8_t *data, uint16_t data_len, 
     const uint8_t *search, uint16_t search_len, bool from_end);

#endif
