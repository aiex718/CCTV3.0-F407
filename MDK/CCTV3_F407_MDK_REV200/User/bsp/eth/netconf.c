/**
 ******************************************************************************
 * @file    netconf.c
 * @author  MCD Application Team
 * @version V1.1.0
 * @date    31-July-2013
 * @brief   Network connection configuration
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
#include "bsp/eth/netconf.h"

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"

#include "lwip/timeouts.h"
#include "lwip/udp.h"

#include "lwip/init.h"
#include "netif/etharp.h"

#include "bsp/eth/lwip_port/Standalone/ethernetif.h"
#include "bsp/eth/stm32f4x7_eth_phy.h"
#include "bsp/sys/dbg_serial.h"

#include "bsp/platform/device/dev_ethernetif.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initializes the lwIP stack
 * @param  None
 * @retval None
 */
void LwIP_Init(void)
{
	// Initialize the LwIP stack
	lwip_init();

	/* In LwIP 2.1.3, all periodic process is handled by sys_timeouts,
	Including tcp_tmr, etharp_tmr, dhcp_fine_tmr....etc,
	user no longer need to call them manually, only sys_check_timeouts() 
	is needed. Refer to lwip/timeouts.c/.h for more details. */

	/* - netif_add(struct netif *netif, ip_addr_t *ipaddr,
	ip_addr_t *netmask, ip_addr_t *gw,
	void *state, err_t (* init)(struct netif *netif),
	err_t (* input)(struct pbuf *p, struct netif *netif))

	Adds your network interface to the netif_list. Allocate a struct
	netif and pass a pointer to this structure as the first argument.
	Give pointers to cleared ip_addr structures when using DHCP,
	or fill them with sane numbers otherwise. The state pointer may be NULL.

	The init function pointer must point to a initialization function for
	your ethernet netif interface. The following code illustrates it's use.*/
	netif_add(&Dev_Ethernetif_Default->_netif, IP4_ADDR_ANY4, IP4_ADDR_ANY4, IP4_ADDR_ANY4,
		Dev_Ethernetif_Default, &Ethernetif_Init, &ethernet_input);

	/*  Registers the default network interface.*/
	netif_set_default(&Dev_Ethernetif_Default->_netif);

	if (ETH_CheckLinkStatus(&Dev_Ethernetif_Default->_netif))
	{
		/* When the netif is fully configured this function must be called.*/
		netif_set_link_up(&Dev_Ethernetif_Default->_netif);
	}
	else
	{
		/*  When the netif link is down this function must be called.*/
		netif_set_link_down(&Dev_Ethernetif_Default->_netif);
		DBG_INFO("Network cable not connected\n");
	}

	/* In LwIP 2.1.3, all periodic process is handled by sys_timeouts,
	 * Including tcp_tmr, etharp_tmr, dhcp_fine_tmr....etc,
	 * user no longer need to call them manually, only sys_check_timeouts() is needed.
	 * Refer to lwip/timeouts.c/.h for more details. */

	/* Set the link callback function, this function is called on change of link status*/
	netif_set_link_callback(&Dev_Ethernetif_Default->_netif, ETH_LinkChanged_callback);
	
	/* call Ethernetif_Service to trigger link status check, if network cable is connected
	ETH_LinkChanged_callback will be invoked to set netif and ip address. */
	Ethernetif_Service(&Dev_Ethernetif_Default->_netif);
}

/**
 * @brief  Called when a frame is received
 * @param  None
 * @retval None
 */
void LwIP_Pkt_Handle(void)
{
	/* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
	Ethernetif_Input(&Dev_Ethernetif_Default->_netif);
}

#if defined ( __CC_ARM )  /* MDK ARM Compiler */
#include "lwip/sio.h"
/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
  sio_fd_t sd;

/* USER CODE BEGIN 7 */
  sd = 0; // dummy code
/* USER CODE END 7 */

  return sd;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
/* USER CODE BEGIN 8 */
/* USER CODE END 8 */
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 9 */
  recved_bytes = 0; // dummy code
/* USER CODE END 9 */
  return recved_bytes;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 10 */
  recved_bytes = 0; // dummy code
/* USER CODE END 10 */
  return recved_bytes;
}
#endif /* MDK ARM Compiler */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
