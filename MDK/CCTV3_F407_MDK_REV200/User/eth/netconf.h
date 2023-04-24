/**
  ******************************************************************************
  * @file    netconf.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013 
  * @brief   This file contains all the functions prototypes for the netconf.c 
  *          file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NETCONF_H
#define __NETCONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "bsp/platform/platform_opts.h"
#include "bsp/sys/dbg_serial.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

//Check phy interval
#ifndef NETCONF_CHECK_LINK_PERIOD
  #define NETCONF_CHECK_LINK_PERIOD 1000
#endif

#ifndef DHCP_EN
  #define DHCP_EN 0
#endif

//TODO:Get UID for mac
//Mac address
#define MAC_ADDR0                     2
#define MAC_ADDR1                     3
#define MAC_ADDR2                     4
#define MAC_ADDR3                     5
#define MAC_ADDR4                     6
#define MAC_ADDR5                     7


#define IP_ADDR0                    192
#define IP_ADDR1                    168
#define IP_ADDR2                     10
#define IP_ADDR3                    100


#define NETMASK_ADDR0               255
#define NETMASK_ADDR1               255
#define NETMASK_ADDR2               255
#define NETMASK_ADDR3                 0


#define GW_ADDR0                      0
#define GW_ADDR1                      0
#define GW_ADDR2                      0
#define GW_ADDR3                      0

/* MII and RMII mode selection ***********/
#define RMII_MODE  
//#define MII_MODE

#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif
	 
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LwIP_Init(void);
void LwIP_Pkt_Handle(void);
void LwIP_Periodic_Handle(__IO uint32_t localtime);

__STATIC_INLINE void print_netif_addr(struct netif *netif)
{
	DBG_INFO("IP  : %s\n",ip4addr_ntoa(&(netif->ip_addr)));
	DBG_INFO("MASK: %s\n",ip4addr_ntoa(&(netif->netmask)));
	DBG_INFO("GW  : %s\n",ip4addr_ntoa(&(netif->gw)));
#if LWIP_DNS
	//TODO:Impl DNS
	printf("dns0:%s\n",ip4addr_ntoa((const ip_addr_t*)dns_getserver(0)));
	printf("dns1:%s\n",ip4addr_ntoa((const ip_addr_t*)dns_getserver(1)));
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
