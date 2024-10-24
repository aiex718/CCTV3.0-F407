/**
	******************************************************************************
	* @file    stm32f4x7_eth_bsp.c
	* @author  MCD Application Team
	* @version V1.1.0
	* @date    31-July-2013 
	* @brief   STM32F4x7 Ethernet hardware configuration.
	******************************************************************************
	* @attention
	*
	* <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
	*
	* Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
	* You may not use this file except in compliance with the License.
	* You may obtain a copy of the License at:
	*
	*        http://www.st.com/software_license_agreement_liberty_v2
	*
	* Unless required by applicable law or agreed to in writing, software 
	* distributed under the License is distributed on an "AS IS" BASIS, 
	* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	* See the License for the specific language governing permissions and
	* limitations under the License.
	*
	******************************************************************************
	*/

/* Includes ------------------------------------------------------------------*/
#include "bsp/platform/platform_defs.h"
#include "bsp/eth/stm32f4x7_eth_phy.h"
#include "bsp/eth/stm32f4x7_eth.h"
#include "bsp/eth/netconf.h"

#include "lwip/opt.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/netif.h"

#include "bsp/sys/dbg_serial.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define GET_PHY_LINK_STATUS()		(ETH_ReadPHYRegister(ETHERNET_PHY_ADDRESS, PHY_BSR) & 0x00000004)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ETH_InitTypeDef ETH_InitStructure;
//extern struct netif gnetif;

/* Private function prototypes -----------------------------------------------*/
static void ETH_GPIO_Config(void);
static void ETH_MACDMA_Config(void);

/* Private functions ---------------------------------------------------------*/

/**
	* @brief  ETH_BSP_Config
	* @param  None
	* @retval None
	*/
void ETH_BSP_Config(void)
{
	/* Configure the GPIO ports for ethernet pins */
	ETH_GPIO_Config();
	
	/* Configure the Ethernet MAC/DMA */
	ETH_MACDMA_Config();

	// /* Configure the PHY to generate an interrupt on change of link status */
	// Eth_Link_PHYITConfig(ETHERNET_PHY_ADDRESS);

	// /* Configure the EXTI for Ethernet link status. */
	// Eth_Link_EXTIConfig(); 
}

/**
	* @brief  Configures the Ethernet Interface
	* @param  None
	* @retval None
	*/
static void ETH_MACDMA_Config(void)
{  
	/* Enable ETHERNET clock  */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
												RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);

	/* Reset ETHERNET on AHB Bus */
	ETH_DeInit();

	/* Software reset */
	ETH_SoftwareReset();

	/* Wait for software reset */
	while (ETH_GetSoftwareResetStatus() == SET);

	/* ETHERNET Configuration --------------------------------------------------*/
	/* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
	ETH_StructInit(&ETH_InitStructure);

	/* Fill ETH_InitStructure parametrs */
	/*------------------------   MAC   -----------------------------------*/
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
//  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
//  ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
//  ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;

	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

	/*------------------------   DMA   -----------------------------------*/  
	
	/* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
	the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
	if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

	/* Configure Ethernet */
	ETH_Init(&ETH_InitStructure, ETHERNET_PHY_ADDRESS);
}

/**
	* @brief  Configures the different GPIO ports.
	* @param  None
	* @retval None
	*/
void ETH_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable GPIOs clocks */
	RCC_AHB1PeriphClockCmd(ETH_MDIO_GPIO_CLK         | ETH_MDC_GPIO_CLK          |
						ETH_RMII_REF_CLK_GPIO_CLK    | ETH_RMII_CRS_DV_GPIO_CLK  |
						ETH_RMII_RXD0_GPIO_CLK       | ETH_RMII_RXD1_GPIO_CLK    |
						ETH_RMII_TX_EN_GPIO_CLK      | ETH_RMII_TXD0_GPIO_CLK    |
						ETH_RMII_TXD1_GPIO_CLK       | ETH_NRST_GPIO_CLK         , ENABLE);

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  

	/* MII/RMII Media interface selection --------------------------------------*/
#ifdef MII_MODE /* Mode MII with STM324xx-EVAL  */
 #ifdef PHY_CLOCK_MCO
	/* Configure MCO (PA8) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Output HSE clock (25MHz) on MCO pin (PA8) to clock the PHY */
	RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
 #endif /* PHY_CLOCK_MCO */

	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_MII);
#elif defined RMII_MODE  /* Mode RMII with STM324xx-EVAL */

	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);
#endif

/* Ethernet pins configuration ************************************************/
	 /*
				ETH_MDIO -------------------------> PA2
				ETH_MDC --------------------------> PC1
				ETH_MII_RX_CLK/ETH_RMII_REF_CLK---> PA1
				ETH_MII_RX_DV/ETH_RMII_CRS_DV ----> PA7
				ETH_MII_RXD0/ETH_RMII_RXD0 -------> PC4
				ETH_MII_RXD1/ETH_RMII_RXD1 -------> PC5
				ETH_MII_TX_EN/ETH_RMII_TX_EN -----> PB11
				ETH_MII_TXD0/ETH_RMII_TXD0 -------> PG13
				ETH_MII_TXD1/ETH_RMII_TXD1 -------> PG14
				ETH_NRST -------------------------> PI1
																									*/

	GPIO_InitStructure.GPIO_Pin = ETH_NRST_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(ETH_NRST_PORT, &GPIO_InitStructure);
	
	ETH_NRST_PIN_LOW();
	_eth_delay_(LAN8742A_RESET_DELAY);
	ETH_NRST_PIN_HIGH();
	_eth_delay_(LAN8742A_RESET_DELAY);
	
	 /* Configure ETH_MDIO A2 v*/
	GPIO_InitStructure.GPIO_Pin = ETH_MDIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ETH_MDIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_MDIO_PORT, ETH_MDIO_SOURCE, ETH_MDIO_AF);
	
	/* Configure ETH_MDC C1 v*/
	GPIO_InitStructure.GPIO_Pin = ETH_MDC_PIN;
	GPIO_Init(ETH_MDC_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_MDC_PORT, ETH_MDC_SOURCE, ETH_MDC_AF);
	
	/* Configure ETH_RMII_REF_CLK A1 v*/
	GPIO_InitStructure.GPIO_Pin = ETH_RMII_REF_CLK_PIN;
	GPIO_Init(ETH_RMII_REF_CLK_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_RMII_REF_CLK_PORT, ETH_RMII_REF_CLK_SOURCE, ETH_RMII_REF_CLK_AF);
	
	/* Configure ETH_RMII_CRS_DV A7 v*/
	GPIO_InitStructure.GPIO_Pin = ETH_RMII_CRS_DV_PIN;
	GPIO_Init(ETH_RMII_CRS_DV_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_RMII_CRS_DV_PORT, ETH_RMII_CRS_DV_SOURCE, ETH_RMII_CRS_DV_AF);
	
	/* Configure ETH_RMII_RXD0 C4 v*/
	GPIO_InitStructure.GPIO_Pin = ETH_RMII_RXD0_PIN;
	GPIO_Init(ETH_RMII_RXD0_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_RMII_RXD0_PORT, ETH_RMII_RXD0_SOURCE, ETH_RMII_RXD0_AF);
	
	/* Configure ETH_RMII_RXD1 C5 v*/
	GPIO_InitStructure.GPIO_Pin = ETH_RMII_RXD1_PIN;
	GPIO_Init(ETH_RMII_RXD1_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_RMII_RXD1_PORT, ETH_RMII_RXD1_SOURCE, ETH_RMII_RXD1_AF);
	
	/* Configure ETH_RMII_TX_EN B11*/
	GPIO_InitStructure.GPIO_Pin = ETH_RMII_TX_EN_PIN;
	GPIO_Init(ETH_RMII_TX_EN_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_RMII_TX_EN_PORT, ETH_RMII_TX_EN_SOURCE, ETH_RMII_TX_EN_AF);
	
	/* Configure ETH_RMII_TXD0 G13 v*/
	GPIO_InitStructure.GPIO_Pin = ETH_RMII_TXD0_PIN;
	GPIO_Init(ETH_RMII_TXD0_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_RMII_TXD0_PORT, ETH_RMII_TXD0_SOURCE, ETH_RMII_TXD0_AF);
	
	/* Configure ETH_RMII_TXD1 G14 v*/
	GPIO_InitStructure.GPIO_Pin = ETH_RMII_TXD1_PIN;
	GPIO_Init(ETH_RMII_TXD1_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_RMII_TXD1_PORT, ETH_RMII_TXD1_SOURCE, ETH_RMII_TXD1_AF);		
}

/**
 * @brief This function should be called periodically.
 *        It checks link status for ethernet,
 * 		  let callback triggered if link status changed.
 */
bool ETH_CheckLinkStatus(struct netif *gnetif)
{
	return GET_PHY_LINK_STATUS()>0;
}

/**
	* @brief  Link callback function, this function is called on change of link status.
	* @param  The network interface
	* @retval None
	*/
void ETH_LinkChanged_callback(struct netif *gnetif)
{
	__IO uint32_t timeout = 0;
 	uint32_t tmpreg,RegValue;

	if(netif_is_link_up(gnetif))
	{
		DBG_INFO("Network cable connected\n");

		/* Restart the autonegotiation */
		if(ETH_InitStructure.ETH_AutoNegotiation != ETH_AutoNegotiation_Disable)
		{
			DBG_INFO("Starting AutoNegotiation...\n");
			/* Reset Timeout counter */
			timeout = 0;

			/* Enable Auto-Negotiation */
			ETH_WritePHYRegister(ETHERNET_PHY_ADDRESS, PHY_BCR, PHY_AutoNegotiation);

			/* Force restart Auto-Negotiation, not necessary(some phy need restart manually) */
			//ETH_WritePHYRegister(ETHERNET_PHY_ADDRESS, PHY_BCR, PHY_Restart_AutoNegotiation);

			/* Wait until the auto-negotiation will be completed */
			do
			{
				timeout++;
			} while (!(ETH_ReadPHYRegister(ETHERNET_PHY_ADDRESS, PHY_BSR) & PHY_AutoNego_Complete) && (timeout < (uint32_t)PHY_READ_TO));

			if(timeout == PHY_READ_TO)
				DBG_INFO("AutoNegotiation Timeout!\n");
			else
				DBG_INFO("AutoNegotiation Done\n");

			/* Reset Timeout counter */
			timeout = 0;

			/* Read the result of the auto-negotiation */
			RegValue = ETH_ReadPHYRegister(ETHERNET_PHY_ADDRESS, PHY_SR);
		
			/* Configure the MAC with the Duplex Mode fixed by the auto-negotiation process */
			if((RegValue & PHY_DUPLEX_STATUS) != (uint32_t)RESET)
			{
				/* Set Ethernet duplex mode to Full-duplex following the auto-negotiation */
				ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;  
			}
			else
			{
				/* Set Ethernet duplex mode to Half-duplex following the auto-negotiation */
				ETH_InitStructure.ETH_Mode = ETH_Mode_HalfDuplex;
			}
			/* Configure the MAC with the speed fixed by the auto-negotiation process */
			if(RegValue & PHY_SPEED_STATUS)
			{
				/* Set Ethernet speed to 10M following the auto-negotiation */
				ETH_InitStructure.ETH_Speed = ETH_Speed_10M; 
			}
			else
			{
				/* Set Ethernet speed to 100M following the auto-negotiation */ 
				ETH_InitStructure.ETH_Speed = ETH_Speed_100M;  
			}

			DBG_INFO("Ethernet Mode %s Duplex %sM\n", 
				(ETH_InitStructure.ETH_Mode == ETH_Mode_FullDuplex) ? "Full" : "Half",
				(ETH_InitStructure.ETH_Speed == ETH_Speed_10M) ? "10" : "100");

			/*------------------------ ETHERNET MACCR Re-Configuration --------------------*/
			/* Get the ETHERNET MACCR value */  
			tmpreg = ETH->MACCR;

			/* Set the FES bit according to ETH_Speed value */ 
			/* Set the DM bit according to ETH_Mode value */ 
			tmpreg |= (uint32_t)(ETH_InitStructure.ETH_Speed | ETH_InitStructure.ETH_Mode);

			/* Write to ETHERNET MACCR */
			ETH->MACCR = (uint32_t)tmpreg;

			_eth_delay_(ETH_REG_WRITE_DELAY);
			tmpreg = ETH->MACCR;
			ETH->MACCR = tmpreg;
		}

		/* Restart MAC interface */
		ETH_Start();
	}
	else
	{
		DBG_INFO("Network cable disconnected\n");

		ETH_Stop();
	}
}


#if 0 //EXTI related
/**
	* @brief  Configure the PHY to generate an interrupt on change of link status.
	* @param PHYAddress: external PHY address  
	* @retval None
	*/
uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress)
{
	uint16_t tmpreg = 0;

	/* Read MICR register */
	tmpreg = ETH_ReadPHYRegister(PHYAddress, PHY_MICR);

	/* Enable output interrupt events to signal via the INT pin */
	tmpreg |= (uint16_t)(PHY_MICR_INT_EN | PHY_MICR_INT_OE);
	if(!(ETH_WritePHYRegister(PHYAddress, PHY_MICR, tmpreg)))
	{
		/* Return ERROR in case of write timeout */
		return ETH_ERROR;
	}

	/* Read MISR register */
	tmpreg = ETH_ReadPHYRegister(PHYAddress, PHY_MISR);

	/* Enable Interrupt on change of link status */
	tmpreg |= (uint16_t)PHY_MISR_LINK_INT_EN;
	if(!(ETH_WritePHYRegister(PHYAddress, PHY_MISR, tmpreg)))
	{
		/* Return ERROR in case of write timeout */
		return ETH_ERROR;
	}
	/* Return SUCCESS */
	return ETH_SUCCESS;   
}

/**
	* @brief  EXTI configuration for Ethernet link status.
	* @param PHYAddress: external PHY address  
	* @retval None
	*/
void Eth_Link_EXTIConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the INT (PB14) Clock */
	RCC_AHB1PeriphClockCmd(ETH_LINK_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure INT pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = ETH_LINK_PIN;
	GPIO_Init(ETH_LINK_GPIO_PORT, &GPIO_InitStructure);

	/* Connect EXTI Line to INT Pin */
	SYSCFG_EXTILineConfig(ETH_LINK_EXTI_PORT_SOURCE, ETH_LINK_EXTI_PIN_SOURCE);

	/* Configure EXTI line */
	EXTI_InitStructure.EXTI_Line = ETH_LINK_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set the EXTI interrupt to priority 1*/
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/**
	* @brief  This function handles Ethernet link status.
	* @param  None
	* @retval None
	*/
void Eth_Link_ITHandler(uint16_t PHYAddress)
{
	/* Check whether the link interrupt has occurred or not */
	if(((ETH_ReadPHYRegister(PHYAddress, PHY_MISR)) & PHY_LINK_STATUS) != 0)
	{
		if((ETH_ReadPHYRegister(PHYAddress, PHY_SR) & 1))
		{
			netif_set_link_up(&gnetif);
		}
		else
		{
			netif_set_link_down(&gnetif);
		}
	}
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
