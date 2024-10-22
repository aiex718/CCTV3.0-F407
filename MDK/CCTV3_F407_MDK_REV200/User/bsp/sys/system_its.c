#include "bsp/platform/platform_defs.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/dbg_serial.h"

#ifndef USE_DEFAULT_SYSFAULT_HANDLER
    #define USE_DEFAULT_SYSFAULT_HANDLER 1
#endif


#if USE_DEFAULT_SYSFAULT_HANDLER

void Dump_Stack(unsigned int *msp_psp,unsigned int *regs, unsigned int lr_value ,char* msg)
{
    DBG_Serial_SafeMode(Peri_DBG_Serial,true);

    DBG_PRINTF("%s\n",msg);
    DBG_PRINTF("Stack dump at %s 0x%08x\n",lr_value&0x04?"PSP":"MSP",msp_psp);
    DBG_PRINTF(" R0  = 0x%08x\n",(unsigned long)msp_psp[0]);
    DBG_PRINTF(" R1  = 0x%08x\n",(unsigned long)msp_psp[1]);
    DBG_PRINTF(" R2  = 0x%08x\n",(unsigned long)msp_psp[2]);
    DBG_PRINTF(" R3  = 0x%08x\n",(unsigned long)msp_psp[3]);
    DBG_PRINTF(" R4  = 0x%08x\n",(unsigned long)regs[0]);
    DBG_PRINTF(" R5  = 0x%08x\n",(unsigned long)regs[1]);
    DBG_PRINTF(" R6  = 0x%08x\n",(unsigned long)regs[2]);
    DBG_PRINTF(" R7  = 0x%08x\n",(unsigned long)regs[3]);
    DBG_PRINTF(" R8  = 0x%08x\n",(unsigned long)regs[4]);
    DBG_PRINTF(" R9  = 0x%08x\n",(unsigned long)regs[5]);
    DBG_PRINTF(" R10 = 0x%08x\n",(unsigned long)regs[6]);
    DBG_PRINTF(" R11 = 0x%08x\n",(unsigned long)regs[7]);
    DBG_PRINTF(" R12 = 0x%08x\n",(unsigned long)msp_psp[4]);
    DBG_PRINTF(" LR  = 0x%08x\n",(unsigned long)msp_psp[5]);
    DBG_PRINTF(" PC  = 0x%08x\n",(unsigned long)msp_psp[6]);
    DBG_PRINTF(" PSR = 0x%08x\n",(unsigned long)msp_psp[7]);    
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

    //store r4-r11 and lr value to stack
    push  {r4-r11}
    //put address that store r4-r11 to r1, as second argument for the C handler
    mrseq  r1, msp

    //put LR value into the r2 register, as third argument for the C handler
    mov    r2, lr

    //padding to make sure the length of this function is 4 bytes
    //nop    

    //put message to r3, as 4th argument for the C handler
    ldr    r3, =__cpp(HardFaultMsg)

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

