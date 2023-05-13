#include "bsp/platform/platform_defs.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/dbg_serial.h"

#ifndef USE_DEFAULT_SYSFAULT_HANDLER
    #define USE_DEFAULT_SYSFAULT_HANDLER 1
#endif


#if USE_DEFAULT_SYSFAULT_HANDLER

void Dump_Stack(unsigned int *hardfault_args, unsigned int lr_value ,char* msg)
{
    unsigned long stacked_r0;
    unsigned long stacked_r1;
    unsigned long stacked_r2;
    unsigned long stacked_r3; 
    unsigned long stacked_r12;
    unsigned long stacked_lr;
    unsigned long stacked_pc;
    unsigned long stacked_psr;   
 
    stacked_r0 =  ((unsigned long)hardfault_args[0]);
    stacked_r1 =  ((unsigned long)hardfault_args[1]);
    stacked_r2 =  ((unsigned long)hardfault_args[2]);
    stacked_r3 =  ((unsigned long)hardfault_args[3]);
    stacked_r12 = ((unsigned long)hardfault_args[4]);
    stacked_lr =  ((unsigned long)hardfault_args[5]);
    stacked_pc =  ((unsigned long)hardfault_args[6]);
    stacked_psr = ((unsigned long)hardfault_args[7]);

    DBG_Serial_SafeMode(Peri_DBG_Serial,true);

    
    DBG_PRINTF("%s\n",msg);
    DBG_PRINTF("Stack dump at MSP\\PSP 0x%08x\n",hardfault_args);
    DBG_PRINTF(" R0  = 0x%08x\n",  stacked_r0);
    DBG_PRINTF(" R1  = 0x%08x\n",  stacked_r1);
    DBG_PRINTF(" R2  = 0x%08x\n",  stacked_r2);
    DBG_PRINTF(" R3  = 0x%08x\n",  stacked_r3);
    DBG_PRINTF(" R12 = 0x%08x\n", stacked_r12);
    DBG_PRINTF(" LR  = 0x%08x\n",  stacked_lr);
    DBG_PRINTF(" PC  = 0x%08x\n",  stacked_pc);
    DBG_PRINTF(" PSR = 0x%08x\n", stacked_psr);    
    DBG_PRINTF(" LR/EXC_RETURN = 0x%08x\n", lr_value);
    //to jump to this function, msp must move forward 4 word=16 bytes,
    //if set breakpoint here, actual msp when exception occurs is  
    //16 bytes less(0x10) than the value shown in debugger
    while(1);
}

/*  When hard fault occurs, the value in lr register can be used to determine
whether the fault happened in main stack(MSP) or process stack(PSP). The original 
lr value is saved in stack. */
const char HardFaultMsg[]="HardFault Handler";
__asm void HardFault_Handler(void)
{
    
    //bitwise AND between the LR and the value 0x04 (binary 0100B)
    //and sets the condition flags based on the result.
    tst    lr, #4 

    //Tests the condition flags set by the previous instruction 
    //(i.e., whether the exception was taken from the main stack
    //or the process stack)
    ite    eq

    //If condition was equal (i.e., the exception was taken from the main stack)
    //put MSP value into the r0 register, as first argument for the C handler
    mrseq  r0, msp
    //If condition was not equal (i.e., the exception was taken from the process stack)
    //put the value of the PSP into the r0 register, as first argument for the C handler
    mrsne  r0, psp

    //put LR value into the r1 register, as second argument for the C handler
    mov    r1, lr

    //padding to make sure the length of this function is 4 bytes
    nop    

    //put message to r2, as third argument for the C handler
    ldr    r2, =__cpp(HardFaultMsg)
    //Branch to the C handler
    b      __cpp(Dump_Stack)
}

void MemManage_Handler(void)
{
    DBG_Serial_SafeMode(Peri_DBG_Serial,true);
    DBG_PRINTF("MemManage_Handler\n");
    while(1);
}

void BusFault_Handler(void)
{
    DBG_Serial_SafeMode(Peri_DBG_Serial,true);
    DBG_PRINTF("BusFault_Handler\n");
    while(1);
}

void UsageFault_Handler(void)
{
    DBG_Serial_SafeMode(Peri_DBG_Serial,true);
    DBG_PRINTF("UsageFault_Handler\n");
    while(1);
}
#endif

void NMI_Handler(void)
{

}

void DebugMon_Handler(void)
{

}

void SVC_Handler(void)
{

}

void PendSV_Handler(void)
{

}

void SysTick_Handler(void)
{
    SysTime_Inc();
}

