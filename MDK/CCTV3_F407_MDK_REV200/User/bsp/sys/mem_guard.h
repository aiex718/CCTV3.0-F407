#ifndef __MEM_GUARD_H__
#define __MEM_GUARD_H__


unsigned int Mem_Guard_Init(void);
int Mem_Guard_CheckOVF(void);
unsigned int Mem_Guard_GetStackAvailable(void);

#endif
