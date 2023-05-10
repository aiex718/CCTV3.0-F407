#include "bsp/platform/device/dev_ethernetif.h"

#include "bsp/eth/stm32f4x7_eth_phy.h"

Ethernetif_t Dev_Ethernetif_Default_Inst= {
    .Netif_IP = 192|168<<8|10<<16|100<<24,
    .Netif_Mask = 255|255<<8|255<<16|0<<24,
    .Netif_Gateway = 0,
#if LWIP_DNS
    .Netif_DNS0 = 8|8<<8|8<<16|8<<24,
    .Netif_DNS1 = 8|8<<8|4<<16|4<<24,
#endif
    .Netif_DHCP_Enable = false,
    .Netif_PHYCheckLink_Func = ETH_CheckLinkStatus,
};

Ethernetif_t *Dev_Ethernetif_Default = &Dev_Ethernetif_Default_Inst;
