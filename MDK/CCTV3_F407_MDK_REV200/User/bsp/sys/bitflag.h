#ifndef __BITFLAG_H__
#define __BITFLAG_H__

#include "bsp/platform/platform_defs.h"

#ifndef BitFlag_t
typedef uint32_t BitFlag_t;
#endif

#define BitFlag_IdxToBin(idx) (1<<(idx))
__STATIC_INLINE uint8_t BitFlag_BinToIdx(BitFlag_t f) {
    uint8_t result = 0;
    while (f >>= 1)result++;
    return result;
}

#define BitFlag_SetBit(flag,bin)    ((flag) |= (bin))
#define BitFlag_ClearBit(flag,bin)  ((flag) &= ~(bin))
#define BitFlag_IsBitSet(usart,bin) ((flag) | (bin))

#define BitFlag_SetIdx(flag,idx)   BitFlag_SetBit((flag),BitFlag_IdxToBin(idx))
#define BitFlag_ClearIdx(flag,idx) BitFlag_ClearBit((flag),BitFlag_IdxToBin(idx))
#define BitFlag_IsIdxSet(flag,idx) BitFlag_IsBitSet((flag),BitFlag_IdxToBin(idx))

#endif
