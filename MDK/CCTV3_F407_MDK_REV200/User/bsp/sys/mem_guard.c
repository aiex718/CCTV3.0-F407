#include "bsp/sys/mem_guard.h"
// require micro.lib 

//Normally we don't need to check heap, it's managed by malloc.
#ifndef MEM_GUARD_CHECK_HEAP
    #define MEM_GUARD_CHECK_HEAP 0
#endif

#ifndef MEM_GUARD_INT_COUNT
    #define MEM_GUARD_INT_COUNT 16 //16*int 
#endif

#ifndef MEM_GUARD_PRINT_INFO
    #define MEM_GUARD_PRINT_INFO 0
#endif

/*
A example of microlib stack and heap mem layout:
----------------------------------------------------------------------------------
    Symbol Name      Value     Ov Type        Size  Object(Section)
----------------------------------------------------------------------------------
    Heap_Mem         0x20000630   Data         512  startup_stm32f40_41xxx.o(HEAP)
    __heap_base      0x20000630   Data           0  startup_stm32f40_41xxx.o(HEAP)
    Stack_Mem        0x20000830   Data        4096  startup_stm32f40_41xxx.o(STACK)
    __heap_limit     0x20000830   Data           0  startup_stm32f40_41xxx.o(HEAP)
    __initial_sp     0x20001830   Data           0  startup_stm32f40_41xxx.o(STACK)
----------------------------------------------------------------------------------
Stack and heap are allocated in the same memory region, and placed back to back,
grow towards each other.

Heap alloc from __heap_base to __heap_limit(which equals to Stack_Mem)
Stack alloc from __initial_sp to __heap_limit.

If you're not using malloc, heap will be optimized out by ARMCC.

A example of microlib stack only mem layout:
----------------------------------------------------------------------------------
    Symbol Name      Value     Ov Type        Size  Object(Section)
----------------------------------------------------------------------------------
    Stack_Mem        0x20000628   Data        4096  startup_stm32f40_41xxx.o(STACK)
    __initial_sp     0x20001628   Data           0  startup_stm32f40_41xxx.o(STACK)
----------------------------------------------------------------------------------

*/

/* Heap */ 
#if MEM_GUARD_CHECK_HEAP
//Put EXPORT __heap_base in startup.s if not exist
extern const unsigned int __heap_base;
//heap_start_addr is the start address of heap
//If heap not used, the value will be 0?
const unsigned int *heap_start_addr = &__heap_base;
#endif

//Put EXPORT Stack_Mem in startup.s if not exist
extern const unsigned int Stack_Mem;
//stack_heap_end_addr is end address of stack and heap (if heap is used)
const unsigned int *stack_heap_end_addr = &Stack_Mem;    

/* Stack */ 
//Put EXPORT __initial_sp in startup.s if not exist
extern const unsigned int __initial_sp;
//stack_start_addr is the start address of stack
const unsigned int *stack_start_addr = &__initial_sp;        

#define MEM_GUARD_INIT_PATTRN 0x21436587
#define MEM_GUARD_STEP 0x11111111

//Put a pattern at the end of stack, return stack size
unsigned int Mem_Guard_Init(void)
{
    unsigned char cnt=MEM_GUARD_INT_COUNT;
    unsigned int *ptr=(unsigned int *)stack_heap_end_addr;
    unsigned int stack_size = (unsigned char*)stack_start_addr - (unsigned char*)stack_heap_end_addr;
#if MEM_GUARD_CHECK_HEAP
    unsigned int heap_size = (unsigned char*)stack_heap_end_addr - (unsigned char*)heap_start_addr;
#endif

#if MEM_GUARD_PRINT_INFO
    printf("Mem_Guard_Init, stack size:0x%x"
#if MEM_GUARD_CHECK_HEAP
    ", heap size:0x%x"
#endif
    "\n",stack_size
#if MEM_GUARD_CHECK_HEAP
    ,heap_size
#endif
    );
#endif

    *ptr = (unsigned int)MEM_GUARD_INIT_PATTRN;

    while(--cnt)
    {
        *(ptr+1) = *ptr + (unsigned int)MEM_GUARD_STEP;
        ptr+=1;
    }

    return stack_size;
}

//Check stack overflow, return 0 if not overflow
int Mem_Guard_CheckOVF(void)
{
    unsigned char cnt=MEM_GUARD_INT_COUNT;
    unsigned int *ptr=(unsigned int *)stack_heap_end_addr;
    
    if(*ptr!=MEM_GUARD_INIT_PATTRN)
    {
        //overflow at stack_heap_end_addr, high probability is heap overflow
        return 1;
    }

    while(--cnt)
    {
        if( *(ptr+1) != *ptr + (unsigned int)MEM_GUARD_STEP)
            return 1;

        ptr+=1;
    }

    return 0;
}

unsigned int Mem_Guard_GetStackAvailable(void)
{
    unsigned int *stack_base=(unsigned int *)stack_heap_end_addr + MEM_GUARD_INT_COUNT;
    unsigned char *ptr=(unsigned char *)stack_base;

    while (*ptr == 0)
        ptr++;

#if MEM_GUARD_PRINT_INFO
    printf("stack depth 0x%p, remaning 0x%x \n",ptr ,ptr - (unsigned char *)stack_base);
#endif

    return ptr - (unsigned char *)stack_base;
}
