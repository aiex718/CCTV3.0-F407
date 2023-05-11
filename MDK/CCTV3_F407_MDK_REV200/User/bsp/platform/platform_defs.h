#ifndef __PLATFORM_DEFS_H__
#define __PLATFORM_DEFS_H__

#include "bsp/platform/platform_opts.h"

//For test in pc environment
#if !defined(VSCODE_INTELLISENSE) && (defined(_WIN32) || defined(_WIN64) || defined(__APPLE__) || defined(__linux__))
#include "stdio.h"
#include "stdint.h"
#include "stddef.h"
#else
#include "stm32f4xx.h"
#include "stdio.h"
#endif
#if defined  (__GNUC__)
#define __BSP_STRUCT_ALIGN __attribute__((aligned(4)))
//#define __BSP_STRUCT_ALIGN __attribute__((packed))
#else
#define __BSP_STRUCT_ALIGN __align(4)
//#define __BSP_STRUCT_ALIGN __packed
#endif

#include "bsp/sys/boolean.h"
#include "bsp/sys/mem_utils.h"
#include "bsp/sys/cast_ptr.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef BSP_MEMCPY 
    #define BSP_MEMCPY memcpy
#endif
#ifndef BSP_MEMMOVE 
    #define BSP_MEMMOVE memmove
#endif
#ifndef BSP_MEMSET 
    #define BSP_MEMSET memset
#endif
#ifndef BSP_MEMCMP 
    #define BSP_MEMCMP memcmp
#endif
#ifndef BSP_MEMSEARCH
    #define BSP_MEMSEARCH mem_search
#endif
#ifndef BSP_STRNCPY 
    #define BSP_STRNCPY strncpy
#endif
#ifndef BSP_STRCMP
    #define BSP_STRCMP strcmp
#endif
#ifndef BSP_STRLEN
    #define BSP_STRLEN strlen
#endif

#ifndef BSP_MALLOC
    #define BSP_MALLOC malloc
#endif
#ifndef BSP_CALLOC
    #define BSP_CALLOC calloc
#endif
#ifndef BSP_FREE
    #define BSP_FREE free
#endif

#ifndef BSP_RAND 
    #define BSP_RAND rand
#endif
#ifndef BSP_SRAND 
    #define BSP_SRAND srand
#endif

#ifndef BSP_MIN 
    #define BSP_MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef BSP_MAX 
    #define BSP_MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef BSP_ABS
    #define BSP_ABS(x)  (((x) < 0) ? -(x) : (x))
#endif

#ifndef BSP_UNUSED_ARG
    #define BSP_UNUSED_ARG(x) (void)(x)
#endif 
#ifndef BSP_ARR_LEN
    #define BSP_ARR_LEN(arr) (sizeof(arr)/sizeof(arr[0]))
#endif
#ifndef BSP_ARR_CLEAR
    #define BSP_ARR_CLEAR(arr) BSP_MEMSET((arr),0,sizeof(arr))
#endif

#ifndef BSP_ARR_STREND
    #define BSP_ARR_STREND(x) (x)[sizeof(x)-1] = '\0'
#endif

#ifndef NULL 
    #define NULL ((void*)0)
#endif
#ifndef __IO
    #define __IO volatile
#endif
#ifndef __STATIC_INLINE 
    #define __STATIC_INLINE static inline
#endif

#ifndef __HW_BUF_SECTION
    #define __HW_BUF_SECTION 
#endif

#endif
