#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "bsp/platform/platform_defs.h"

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/ip.h"

#include "bsp/sys/dbg_serial.h"

#define ETHERNETIF_IPV4_CHAR_LEN 16

#ifndef ETHERNETIF_SERVICE_PERIOD
#define ETHERNETIF_SERVICE_PERIOD 1000
#endif

// Mac address
#define MAC_ADDR0 0x00
#define MAC_ADDR1 'a'
#define MAC_ADDR2 'c'

/* Network interface name */
#define IFNAME0 'i'
#define IFNAME1 'f'

typedef struct Ethernetif_ConfigFile_s
{
    bool Netif_Config_DHCP_Enable;
    uint8_t __padding[3];
    char Netif_Config_IP[ETHERNETIF_IPV4_CHAR_LEN];
    char Netif_Config_Mask[ETHERNETIF_IPV4_CHAR_LEN];
    char Netif_Config_Gateway[ETHERNETIF_IPV4_CHAR_LEN];
#if LWIP_DNS
    char Netif_Config_DNS0[ETHERNETIF_IPV4_CHAR_LEN];
    char Netif_Config_DNS1[ETHERNETIF_IPV4_CHAR_LEN];
#endif
}Ethernetif_ConfigFile_t;

typedef struct Ethernetif_s
{
    ip4_addr_t Netif_IP;
    ip4_addr_t Netif_Mask;
    ip4_addr_t Netif_Gateway;
#if LWIP_DNS
    ip4_addr_t Netif_DNS0;
    ip4_addr_t Netif_DNS1;
#endif
    //function ptr
    bool (*Netif_PHYCheckLink_Func)(struct netif *netif);
    bool Netif_DHCP_Enable;
    //private
    bool _link_status_prev;
    u8_t _dhcp_state_prev;
    ip4_addr_t _ip_prev,_mask_prev,_gw_prev;
    struct netif _netif;
}Ethernetif_t;

err_t Ethernetif_Init(struct netif *netif);
err_t Ethernetif_Input(struct netif *netif);

void Ethernetif_ConfigSet(Ethernetif_t *self,const Ethernetif_ConfigFile_t *config);
void Ethernetif_ConfigExport(const Ethernetif_t *self,Ethernetif_ConfigFile_t *config);
bool Ethernetif_IsConfigValid(Ethernetif_t *self,const Ethernetif_ConfigFile_t *config);

void Ethernetif_Service(void* netif);


#endif
