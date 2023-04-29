#include "bsp/platform/periph/peri_uniqueid.h"

const HAL_UniqueID_t Periph_UniqueID_Inst = {
	.UniqueID_Addrs = __CONST_ARRAY_CAST_VAR(uint32_t){
		0x1FFF7A10, 0x1FFF7A14, 0x1FFF7A18
	},
	.UniqueID_Len = 3,
};

const HAL_UniqueID_t *Periph_UniqueID = &Periph_UniqueID_Inst; 

