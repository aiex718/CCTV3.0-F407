#include "bsp/hal/rcc.h"
#include "bsp/hal/gpio.h"


void HAL_RCC_Cmd(const HAL_RCC_Cmd_t *cmd, const bool en)
{
    if (cmd->RCC_AHB1Periph)
        RCC_AHB1PeriphClockCmd(cmd->RCC_AHB1Periph, en ? ENABLE : DISABLE);
    if (cmd->RCC_AHB2Periph)
        RCC_AHB2PeriphClockCmd(cmd->RCC_AHB2Periph, en ? ENABLE : DISABLE);
    if (cmd->RCC_AHB3Periph)
        RCC_AHB3PeriphClockCmd(cmd->RCC_AHB3Periph, en ? ENABLE : DISABLE);
    if (cmd->RCC_APB1Periph)
        RCC_APB1PeriphClockCmd(cmd->RCC_APB1Periph, en ? ENABLE : DISABLE);
    if (cmd->RCC_APB2Periph)
        RCC_APB2PeriphClockCmd(cmd->RCC_APB2Periph, en ? ENABLE : DISABLE);
}

void HAL_RCC_Init(HAL_RCC_t *self)
{
    HAL_RCC_MCO_t **mco_list = self->RCC_mco_list;
    HAL_RCC_MCO_t *mco;
    HAL_RCC_CLK_t **clk_list = self->RCC_clk_list;
    HAL_RCC_CLK_t *clk;

    while (*clk_list != NULL)
    {
        clk = *clk_list;
        switch (clk->CLK_Idx)
        {
        case CLK_HSE:
            if (clk->CLK_Enable)
            {
                RCC_HSEConfig(RCC_HSE_ON);
                while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
                    ;
            }
            else
                RCC_HSEConfig(RCC_HSE_OFF);
            break;
        case CLK_HSI:
            if (clk->CLK_Enable)
            {
                RCC_HSICmd(ENABLE);
                while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
                    ;
            }
            else
                RCC_HSICmd(DISABLE);
            break;
        case CLK_LSE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
            PWR_BackupAccessCmd(ENABLE);
            if (clk->CLK_Enable)
            {
                RCC_LSEConfig(RCC_LSE_ON);
                while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
                    ;
            }
            else
                RCC_LSEConfig(RCC_LSE_OFF);

            PWR_BackupAccessCmd(DISABLE);
            break;
        case CLK_LSI:
            if (clk->CLK_Enable)
            {
                RCC_LSICmd(ENABLE);
                while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
                    ;
            }
            else
                RCC_LSICmd(DISABLE);
            break;
        default:
            break;
        }
        clk_list++;
    }

    while (*mco_list != NULL)
    {
        mco = *mco_list;
        switch (mco->MCO_Idx)
        {
        case MCO1:
            RCC_MCO1Config(mco->MCO_Source, mco->MCO_ClkDiv);
            break;
        case MCO2:
            RCC_MCO2Config(mco->MCO_Source, mco->MCO_ClkDiv);
            break;
        default:
            break;
        }
        HAL_GPIO_InitPin(mco->MCO_Pin);

        mco_list++;
    }
}
