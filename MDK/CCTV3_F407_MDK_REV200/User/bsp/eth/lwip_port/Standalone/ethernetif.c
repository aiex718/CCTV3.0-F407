/**
 * @file
 * Ethernet Interface for standalone applications (without RTOS) - works only for
 * ethernet polling mode (polling for ethernet frame reception)
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "bsp/platform/platform_defs.h"
#include "bsp/platform/periph/peri_uniqueid.h"

#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"
#include "lwip/timeouts.h"
#if LWIP_DNS
#include "lwip/dns.h"
#endif

#include "Standalone/ethernetif.h"

#include "bsp/eth/stm32f4x7_eth.h"
#include "bsp/eth/netconf.h"

#include "bsp/platform/periph/peri_uniqueid.h"

/* Ethernet Rx & Tx DMA Descriptors */
extern ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];

/* Ethernet Driver Receive buffers  */
extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];

/* Ethernet Driver Transmit buffers */
extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];

/* Global pointers to track current transmit and receive descriptors */
extern ETH_DMADESCTypeDef *DMATxDescToSet;
extern ETH_DMADESCTypeDef *DMARxDescToGet;

/* Global pointer for last received frame infos */
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;

// services
static void Ethernetif_SetFallbackIpAddr(Ethernetif_t *self);
static void Ethernetif_CheckIPChanged(Ethernetif_t *self);
static void Ethernetif_CheckDHCPChanged(Ethernetif_t *self);
static void Ethernetif_CheckLink(Ethernetif_t *self);

/**
 * In this function, the hardware should be initialized.
 * Called from Ethernetif_Init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
#ifdef CHECKSUM_BY_HARDWARE
    int i;
#endif
    uint32_t uid = 0;
    uint8_t uid_cnt = HAL_UniqueID_GetLen(Periph_UniqueID);
    while (uid_cnt)
        uid ^= HAL_UniqueID_Read(Periph_UniqueID, uid_cnt--);

    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    netif->hwaddr[0] = MAC_ADDR0;
    netif->hwaddr[1] = MAC_ADDR1;
    netif->hwaddr[2] = MAC_ADDR2;
    netif->hwaddr[3] = (uid >> 16) & 0xFF;
    netif->hwaddr[4] = (uid >> 8) & 0xFF;
    netif->hwaddr[5] = uid & 0xFF;

    /* initialize MAC address in ethernet MAC */
    ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr);

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    /* Initialize Tx Descriptors list: Chain Mode */
    ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
    /* Initialize Rx Descriptors list: Chain Mode  */
    ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

#ifdef CHECKSUM_BY_HARDWARE
    /* Enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
    for (i = 0; i < ETH_TXBUFNB; i++)
    {
        ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
    }
#endif

    /* Note: TCP, UDP, ICMP checksum checking for received frame are enabled in DMA config */

    /* Enable MAC and DMA transmission and reception */
    ETH_Start();
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    err_t errval;
    struct pbuf *q;
    u8 *buffer = (u8 *)(DMATxDescToSet->Buffer1Addr);
    __IO ETH_DMADESCTypeDef *DmaTxDesc;
    uint16_t framelength = 0;
    uint32_t bufferoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t payloadoffset = 0;

    DmaTxDesc = DMATxDescToSet;
    bufferoffset = 0;

    /* copy frame from pbufs to driver buffers */
    for (q = p; q != NULL; q = q->next)
    {
        /* Is this buffer available? If not, goto error */
        if ((DmaTxDesc->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
        {
            errval = ERR_BUF;
            goto error;
        }

        /* Get bytes in current lwIP buffer */
        byteslefttocopy = q->len;
        payloadoffset = 0;

        /* Check if the length of data to copy is bigger than Tx buffer size*/
        while ((byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE)
        {
            /* Copy data to Tx buffer*/
            MEMCPY((u8_t *)((u8_t *)buffer + bufferoffset), (u8_t *)((u8_t *)q->payload + payloadoffset), (ETH_TX_BUF_SIZE - bufferoffset));

            /* Point to next descriptor */
            DmaTxDesc = (ETH_DMADESCTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);

            /* Check if the buffer is available */
            if ((DmaTxDesc->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
            {
                errval = ERR_USE;
                goto error;
            }

            buffer = (u8 *)(DmaTxDesc->Buffer1Addr);

            byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
            payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
            framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
            bufferoffset = 0;
        }

        /* Copy the remaining bytes */
        MEMCPY((u8_t *)((u8_t *)buffer + bufferoffset), (u8_t *)((u8_t *)q->payload + payloadoffset), byteslefttocopy);
        bufferoffset = bufferoffset + byteslefttocopy;
        framelength = framelength + byteslefttocopy;
    }

    /* Note: padding and CRC for transmitted frame
       are automatically inserted by DMA */

    /* Prepare transmit descriptors to give to DMA*/
    ETH_Prepare_Transmit_Descriptors(framelength);

    errval = ERR_OK;

error:

    /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
    if ((ETH->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET)
    {
        /* Clear TUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_TUS;

        /* Resume DMA transmission*/
        ETH->DMATPDR = 0;
    }
    return errval;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *low_level_input(struct netif *netif)
{
    struct pbuf *p, *q;
    uint32_t len;
    FrameTypeDef frame;
    u8 *buffer;
    __IO ETH_DMADESCTypeDef *DMARxDesc;
    uint32_t bufferoffset = 0;
    uint32_t payloadoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t i = 0;

    /* get received frame */
    frame = ETH_Get_Received_Frame();

    /* Obtain the size of the packet and put it into the "len" variable. */
    len = frame.length;
    buffer = (u8 *)frame.buffer;

    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

    if (p != NULL)
    {
        DMARxDesc = frame.descriptor;
        bufferoffset = 0;
        for (q = p; q != NULL; q = q->next)
        {
            byteslefttocopy = q->len;
            payloadoffset = 0;

            /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
            while ((byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE)
            {
                /* Copy data to pbuf*/
                MEMCPY((u8_t *)((u8_t *)q->payload + payloadoffset), (u8_t *)((u8_t *)buffer + bufferoffset), (ETH_RX_BUF_SIZE - bufferoffset));

                /* Point to next descriptor */
                DMARxDesc = (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
                buffer = (unsigned char *)(DMARxDesc->Buffer1Addr);

                byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
                payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
                bufferoffset = 0;
            }
            /* Copy remaining data in pbuf */
            MEMCPY((u8_t *)((u8_t *)q->payload + payloadoffset), (u8_t *)((u8_t *)buffer + bufferoffset), byteslefttocopy);
            bufferoffset = bufferoffset + byteslefttocopy;
        }
    }

    /* Release descriptors to DMA */
    DMARxDesc = frame.descriptor;

    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i = 0; i < DMA_RX_FRAME_infos->Seg_Count; i++)
    {
        DMARxDesc->Status = ETH_DMARxDesc_OWN;
        DMARxDesc = (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
    }

    /* Clear Segment_Count */
    DMA_RX_FRAME_infos->Seg_Count = 0;

    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)
    {
        /* Clear RBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        ETH->DMARPDR = 0;
    }
    return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t Ethernetif_Input(struct netif *netif)
{
    err_t err;
    struct pbuf *p;

    /* move received packet into a new pbuf */
    p = low_level_input(netif);

    /* no packet could be read, silently ignore this */
    if (p == NULL)
        return ERR_MEM;

    /* entry point to the LwIP stack */
    err = netif->input(p, netif);

    if (err != ERR_OK)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("Ethernetif_Input: IP input error\n"));
        pbuf_free(p);
    }
    return err;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t Ethernetif_Init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    /* initialize the hardware */
    low_level_init(netif);

    netif_set_up(netif);
    // set static ip or dhcp (if enabled)
    do
    {
        Ethernetif_t *ethernetif = (Ethernetif_t *)netif->state;

        if (ethernetif == NULL)
        {
            DBG_ERROR("Ethernetif_Init: 0x%p netif->state is NULL\n", netif);
            break;
        }

        if (ethernetif->Netif_DHCP_Enable)
        {
            DBG_INFO("Netif 0x%p using dhcp\n", netif);
            Ethernetif_SetFallbackIpAddr(ethernetif);
            //Lwip DHCP module will handle link up/down event automatically
            dhcp_start(netif);
        }
        else
        {
            DBG_INFO("Netif 0x%p using static IP address\n", netif);
#if LWIP_DNS
            dns_setserver(0, &ethernetif->Netif_DNS0);
            dns_setserver(1, &ethernetif->Netif_DNS1);
#endif
            netif_set_addr(netif, &ethernetif->Netif_IP, &ethernetif->Netif_Mask, &ethernetif->Netif_Gateway);
        }
    } while (0);

    return ERR_OK;
}

void Ethernetif_ConfigSet(Ethernetif_t *self, const Ethernetif_ConfigFile_t *config)
{
    if (config->Netif_Config_DHCP_Enable == false)
    {
        ip4_addr_t temp;

#if LWIP_DNS
        if (ipaddr_aton(config->Netif_Config_DNS0, &temp))
            ip_addr_copy(self->Netif_DNS0, temp);
        else
            DBG_ERROR("Netif 0x%p bad DNS0 %s\n", &self->_netif, config->Netif_Config_DNS0);

        if (ipaddr_aton(config->Netif_Config_DNS1, &temp))
            ip_addr_copy(self->Netif_DNS1, temp);
        else
            DBG_ERROR("Netif 0x%p bad DNS1 %s\n", &self->_netif, config->Netif_Config_DNS1);
#endif
        if (ipaddr_aton(config->Netif_Config_IP, &temp))
            ip_addr_copy(self->Netif_IP, temp);
        else
            DBG_ERROR("Netif 0x%p bad IP %s\n", &self->_netif, config->Netif_Config_IP);

        if (ipaddr_aton(config->Netif_Config_Mask, &temp))
            ip_addr_copy(self->Netif_Mask, temp);
        else
            DBG_ERROR("Netif 0x%p bad Mask %s\n", &self->_netif, config->Netif_Config_Mask);

        if (ipaddr_aton(config->Netif_Config_Gateway, &temp))
            ip_addr_copy(self->Netif_Gateway, temp);
        else
            DBG_ERROR("Netif 0x%p bad Gateway %s\n", &self->_netif, config->Netif_Config_Gateway);
    }

    self->Netif_DHCP_Enable = config->Netif_Config_DHCP_Enable;
}

void Ethernetif_ConfigExport(const Ethernetif_t *self, Ethernetif_ConfigFile_t *config)
{
    ipaddr_ntoa_r(&self->Netif_IP, config->Netif_Config_IP, sizeof(config->Netif_Config_IP));
    ipaddr_ntoa_r(&self->Netif_Mask, config->Netif_Config_Mask, sizeof(config->Netif_Config_Mask));
    ipaddr_ntoa_r(&self->Netif_Gateway, config->Netif_Config_Gateway, sizeof(config->Netif_Config_Gateway));
#if LWIP_DNS
    ipaddr_ntoa_r(&self->Netif_DNS0, config->Netif_Config_DNS0, sizeof(config->Netif_Config_DNS0));
    ipaddr_ntoa_r(&self->Netif_DNS1, config->Netif_Config_DNS1, sizeof(config->Netif_Config_DNS1));
#endif
    config->Netif_Config_DHCP_Enable = self->Netif_DHCP_Enable;
}

bool Ethernetif_IsConfigValid(Ethernetif_t *self, const Ethernetif_ConfigFile_t *config)
{
    if (config == NULL)
        return false;
    else if (config->Netif_Config_DHCP_Enable)
        return true;
    else
    {
        ip4_addr_t temp;
        return (ipaddr_aton(config->Netif_Config_IP, &temp) &&
                ip_addr_isany(&temp) == false &&
                ipaddr_aton(config->Netif_Config_Mask, &temp) &&
                ipaddr_aton(config->Netif_Config_Gateway, &temp));
    }
}

// arg is struct netif *
void Ethernetif_Service(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    Ethernetif_t *ethernetif = (Ethernetif_t *)netif->state;

    Ethernetif_CheckDHCPChanged(ethernetif);
    Ethernetif_CheckIPChanged(ethernetif);
    Ethernetif_CheckLink(ethernetif);

    sys_timeout(ETHERNETIF_SERVICE_PERIOD, Ethernetif_Service, arg);
}

// Services
static void Ethernetif_SetFallbackIpAddr(Ethernetif_t *self)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    uint32_t uid = 0;
    uint8_t uid_cnt;

    uid_cnt = HAL_UniqueID_GetLen(Periph_UniqueID);
    while (uid_cnt)
        uid ^= HAL_UniqueID_Read(Periph_UniqueID, uid_cnt--);

    // set a fall back ip address
    IP4_ADDR(&ipaddr, 169, 254, (uid >> 8) & 0xff, uid & 0xff);
    IP4_ADDR(&netmask, 255, 255, 0, 0);
    IP4_ADDR(&gw, 0, 0, 0, 0);

    netif_set_addr(&self->_netif, &ipaddr, &netmask, &gw);
}

static void Ethernetif_CheckIPChanged(Ethernetif_t *self)
{
    struct netif *_netif = &self->_netif;

    if (_netif->ip_addr.addr != self->_ip_prev.addr ||
        _netif->netmask.addr != self->_mask_prev.addr ||
        _netif->gw.addr != self->_gw_prev.addr)
    {
        self->_ip_prev.addr = _netif->ip_addr.addr;
        self->_mask_prev.addr = _netif->netmask.addr;
        self->_gw_prev.addr = _netif->gw.addr;

        DBG_INFO("Netif 0x%p IP address changed\n", _netif);
        Ethernetif_PrintIP(self);
    }
}

static void Ethernetif_CheckDHCPChanged(Ethernetif_t *self)
{
    if (self->Netif_DHCP_Enable == false)
        return;

    struct netif *_netif = &self->_netif;
    u8_t new_state = 0;
    new_state = dhcp_supplied_address(_netif);
    if (new_state != self->_dhcp_state_prev)
    {
        self->_dhcp_state_prev = new_state;
        if (new_state)
            DBG_INFO("Netif 0x%p DHCP new_state %d\n", _netif, new_state);
    }
}

static void Ethernetif_CheckLink(Ethernetif_t *self)
{
    if (self->Netif_PHYCheckLink_Func)
    {
        bool link_status_new = self->Netif_PHYCheckLink_Func(&self->_netif);

        if (link_status_new != self->_link_status_prev)
        {
            self->_link_status_prev = link_status_new;

            if (link_status_new)
                netif_set_link_up(&self->_netif);
            else
                netif_set_link_down(&self->_netif);
        }
    }
    else
    {
        DBG_ERROR("Netif 0x%p no PHYCheckLink_Func\n", &self->_netif);
    }
}

static void Ethernetif_PrintIP(Ethernetif_t *self)
{
    DBG_INFO("IP  : %s\n", ip4addr_ntoa(&(self->_netif.ip_addr)));
    DBG_INFO("MASK: %s\n", ip4addr_ntoa(&(self->_netif.netmask)));
    DBG_INFO("GW  : %s\n", ip4addr_ntoa(&(self->_netif.gw)));
#if LWIP_DNS
    DBG_INFO("DNS0: %s\n", ip4addr_ntoa((const ip_addr_t *)dns_getserver(0)));
    DBG_INFO("DNS1: %s\n", ip4addr_ntoa((const ip_addr_t *)dns_getserver(1)));
#endif
}
