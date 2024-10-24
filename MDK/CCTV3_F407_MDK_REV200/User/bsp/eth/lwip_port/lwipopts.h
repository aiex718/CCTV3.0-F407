/**
  ******************************************************************************
  * @file    lwipopts.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   lwIP Options Configuration.
  *          This file is based on Utilities\lwip_v1.4.1\src\include\lwip\opt.h 
  *          and contains the lwIP configuration for the STM32F4x7 demonstration.
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

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "bsp/sys/systime.h"
#include "app/nettime/nettime.h" //For sntp callback
#include "bsp/platform/periph/peri_rtc.h" //For MJPEGD_GET_UNIX_TIMESTAMP

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT    0

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  1

/**
 * NO_SYS_NO_TIMERS==1: Drop support for sys_timeout when NO_SYS==1
 * Mainly for compatibility to old versions.
 */
#define NO_SYS_NO_TIMERS        0

#define sys_now SysTime_Get

/* RAND */
#define LWIP_RAND()     (u32_t)BSP_RAND()

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (10*1024)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           36
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        4
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        16
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 4
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        20
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    20


/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
//PBUF_POOL mainly for rx
//since mjpegd rarely use rx, we can reduce the size of Pbuf_Pool
#define PBUF_POOL_SIZE          8

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       500


/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
#define TCP_MSS                 (1500 - 40)/* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (4*TCP_MSS)

/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
  as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */

#define TCP_SND_QUEUELEN        (2* TCP_SND_BUF/TCP_MSS)

/* TCP receive window. */
#define TCP_WND                 (2*TCP_MSS)


/* ---------- ICMP options ---------- */
#define LWIP_ICMP                       1


/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP               1

/* DNS options */
#define LWIP_DNS                1

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255


/* ---------- Statistics options ---------- */
#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO 1

/* ---------- link callback options ---------- */
/* LWIP_NETIF_LINK_CALLBACK==1: Support a callback function from an interface
 * whenever the link changes (i.e., link down)
 */
#define LWIP_NETIF_LINK_CALLBACK        1

#define LWIP_SINGLE_NETIF 1

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

/* 
The STM32F4x7 allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
#define CHECKSUM_BY_HARDWARE 


#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0 
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#endif


/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    0

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     0

#define LWIP_HTTPD_SUPPORT_REQUESTLIST 1
#define LWIP_HTTPD_SUPPORT_V09         0
#define LWIP_HTTPD_CGI_SSI             1

#define LWIP_HTTPD_FS_ASYNC_READ       0
#define LWIP_HTTPD_DYNAMIC_FILE_READ   1
#define LWIP_HTTPD_CUSTOM_FILES        1
#define HTTPD_USE_CUSTOM_FSDATA 1

/*
   ----------------------------------------
   ---------- Lwip Debug options ----------
   ----------------------------------------
*/
#define LWIP_DEBUG                     1

// #define MEMP_DEBUG                     LWIP_DBG_ON
// #define MEM_DEBUG                      LWIP_DBG_ON
// #define PBUF_DEBUG                     LWIP_DBG_ON
// #define TCP_DEBUG                      LWIP_DBG_ON

// #define TCP_OUTPUT_DEBUG               LWIP_DBG_ON
// #define TCP_INPUT_DEBUG                LWIP_DBG_ON
// #define TCP_FR_DEBUG                   LWIP_DBG_ON
// #define TCP_RTO_DEBUG                  LWIP_DBG_ON
// #define TCP_CWND_DEBUG                 LWIP_DBG_ON
// #define TCP_WND_DEBUG                  LWIP_DBG_ON
// #define TCP_RST_DEBUG                  LWIP_DBG_ON
// #define TCP_QLEN_DEBUG                 LWIP_DBG_ON


// #define IP_DEBUG                       LWIP_DBG_ON
// #define NETIF_DEBUG                    LWIP_DBG_ON
// #define RAW_DEBUG                      LWIP_DBG_ON
// #define TCPIP_DEBUG                    LWIP_DBG_ON

//#define TIMERS_DEBUG                    (LWIP_DBG_ON|LWIP_DBG_LEVEL_SEVERE|LWIP_DBG_STATE|LWIP_DBG_TRACE)
//#define LWIP_DEBUG_TIMERNAMES            LWIP_DBG_ON

//#define DHCP_DEBUG                       LWIP_DBG_ON
//#define HTTPD_DEBUG                      LWIP_DBG_ON
#define MJPEGD_DEBUG                       LWIP_DBG_ON
#define HTTPD_DEBUG                        LWIP_DBG_ON
#define MJPEGD_FRAMEPOOL_DEBUG             LWIP_DBG_ON
#define WEBHOOK_DEBUG                      LWIP_DBG_ON

#define LWIP_DBG_MIN_LEVEL       LWIP_DBG_LEVEL_ALL//LWIP_DBG_LEVEL_WARNING //LWIP_DBG_LEVEL_SEVERE 
#define LWIP_DBG_TYPES_ON        (LWIP_DBG_LEVEL_SEVERE|LWIP_DBG_STATE)//|LWIP_DBG_TRACE)

//Mjpegd options
#define MJPEGD_SERVICE_PERIOD 10//ms
#define MJPEGD_ALLOW_STREAM_CORS 1
#define MJPEGD_SHOWFPS_PERIOD 3//n^2 seconds
#define MJPEGD_GET_UNIX_TIMESTAMP (u32_t)HAL_RTC_GetTime(Peri_RTC)
#define MJPEGD_STREAM_CLIENT_LIMIT 5
#define MJPEGD_FRAMEPOOL_LEN (MJPEGD_STREAM_CLIENT_LIMIT+1)


//SNTP options
#define SNTP_SET_SYSTEM_TIME NetTime_Sntp_Poll_Callback
#define SNTP_SERVER_DNS 1
//#define SNTP_DEBUG_TRACE LWIP_DBG_ON

#endif /* __LWIPOPTS_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
