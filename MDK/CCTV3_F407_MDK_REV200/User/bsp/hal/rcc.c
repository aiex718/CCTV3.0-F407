#include "bsp/hal/rcc.h"

void HAL_RCC_Cmd(const HAL_RCC_Cmd_t* cmd,const bool en)
{
    if(cmd->RCC_AHB1Periph)
        RCC_AHB1PeriphClockCmd(cmd->RCC_AHB1Periph,en?ENABLE:DISABLE);
    if(cmd->RCC_AHB2Periph)
        RCC_AHB2PeriphClockCmd(cmd->RCC_AHB2Periph,en?ENABLE:DISABLE);
    if(cmd->RCC_AHB3Periph)
        RCC_AHB3PeriphClockCmd(cmd->RCC_AHB3Periph,en?ENABLE:DISABLE);
    if(cmd->RCC_APB1Periph)
        RCC_APB1PeriphClockCmd(cmd->RCC_APB1Periph,en?ENABLE:DISABLE);
    if(cmd->RCC_APB2Periph)
        RCC_APB2PeriphClockCmd(cmd->RCC_APB2Periph,en?ENABLE:DISABLE);
}

