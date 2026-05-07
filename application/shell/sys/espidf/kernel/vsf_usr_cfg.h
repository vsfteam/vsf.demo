/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/


//! \note User Level Application Configuration

#ifndef __VSF_USR_CFG_H__
#define __VSF_USR_CFG_H__

/*============================ INCLUDES ======================================*/

#ifdef __cplusplus
extern "C" {
#endif
extern void vsf_trace_assert(const char *expr, const char *file, int line, const char *func);
#ifdef __cplusplus
}
#endif

// re-write VSF_ASSERT to bypass __EXPR string
#define VSF_ASSERT(__EXPR)                                                      \
    ((__EXPR)                                                                   \
    ? (void)0                                                                   \
    : vsf_trace_assert("", __FILE__, __LINE__, __FUNCTION__))

#include "vsf_board_cfg.h"

// for compiler related configuration, eg: __VSF64__, __IS_COMPILER_XXXX__, etc
#define __VSF_HEADER_ONLY_SHOW_COMPILER_INFO__
#include "utilities/compiler/compiler.h"

/*============================ MACROS ========================================*/

// kernel
#define VSF_OS_CFG_EDA_FRAME_POOL_SIZE                  32
// define VSF_OS_CFG_EDA_FRAME_POOL_EXTRA_SIZE to 16 * sizeof(uintptr_t)
#ifdef __VSF64__
#   define VSF_OS_CFG_EDA_FRAME_POOL_EXTRA_SIZE         (16 * 8)
#else
#   define VSF_OS_CFG_EDA_FRAME_POOL_EXTRA_SIZE         (16 * 4)
#endif
#define VSF_OS_CFG_MAIN_MODE                            VSF_OS_CFG_MAIN_MODE_IDLE

// components

#define VSF_USE_JSON                                    ENABLED
#define VSF_USE_LITTLEFS                                ENABLED
#define VSF_USE_TRACE                                   ENABLED
#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_FIFO                                    ENABLED
#define VSF_USE_SCSI                                    ENABLED
#   define VSF_SCSI_USE_VIRTUAL_SCSI                    ENABLED
#   define VSF_SCSI_USE_MAL_SCSI                        ENABLED
#define VSF_USE_MAL                                     ENABLED
#   define VSF_MAL_USE_SDMMC_MAL                        ENABLED
#   define VSF_MAL_USE_FAKEFAT32_MAL                    ENABLED
#   define VSF_MAL_USE_SCSI_MAL                         ENABLED
#   define VSF_MAL_USE_FLASH_MAL                        ENABLED
#   define VSF_MAL_USE_MEM_MAL                          ENABLED
#   define VSF_MAL_USE_MIM_MAL                          ENABLED
#   define VSF_MAL_USE_FILE_MAL                         ENABLED
#define VSF_USE_FS                                      ENABLED
#   define VSF_FS_USE_ROMFS                             ENABLED
#   define VSF_FS_USE_MEMFS                             ENABLED
#   define VSF_FS_USE_FATFS                             ENABLED
#   define VSF_FS_USE_EXFATFS                           ENABLED
#   define VSF_FS_USE_LITTLEFS                          VSF_USE_LITTLEFS
#define VSF_USE_INPUT                                   ENABLED

#if VSF_BOARD_HAS_USB_HOST
#   define VSF_USE_USB_HOST                             ENABLED
#endif
#if VSF_BOARD_HAS_USB_DEVICE
#   define VSF_USE_USB_DEVICE                           ENABLED
#endif
#if VSF_BOARD_HAS_BLUETOOTH
#   define VSF_USE_BTSTACK                              ENABLED
#endif

#if VSF_USE_USB_DEVICE == ENABLED
#   define VSF_USBD_USE_MSC                             ENABLED
#   define VSF_USBD_USE_CDC                             ENABLED
#       define VSF_USBD_USE_CDCACM                      ENABLED
#       if defined(VSF_USE_LWIP) && VSF_USE_LWIP != ENABLED
#           define VSF_USBD_USE_CDCNCM                  DISABLED
#       else
#           define VSF_USBD_USE_CDCNCM                  ENABLED
#       endif
#endif
#if VSF_USE_USB_HOST == ENABLED
#   define VSF_USBH_USE_LIBUSB                          ENABLED
#   define VSF_USBH_USE_CDC                             ENABLED
#   define VSF_USBH_USE_ECM                             ENABLED
#   define VSF_USBH_USE_NCM                             ENABLED
#   define VSF_USBH_USE_HID                             ENABLED
#   define VSF_USBH_USE_DS4                             ENABLED
#   define VSF_USBH_USE_DS5                             ENABLED
#   define VSF_USBH_USE_NSPRO                           ENABLED
#   define VSF_USBH_USE_XB360                           ENABLED
#   define VSF_USBH_USE_XB1                             ENABLED
#   define VSF_USBH_USE_MSC                             ENABLED
#   define VSF_USBH_USE_UAC                             ENABLED
#   define VSF_USBH_USE_UVC                             ENABLED
#   define VSF_USBH_USE_BTHCI                           ENABLED
#endif

// Force-enable lwIP on host (Windows/Linux) for esp_netif + wpcap demo.
// The original condition only enables lwIP when USB-CDC-NCM is active,
// which is not the case for the vc.espidf project.
#ifndef VSF_USE_LWIP
#   if defined(__WIN__) || defined(__LINUX__)
#       define VSF_USE_LWIP                             ENABLED
#   else
#       define VSF_USE_LWIP                             (VSF_USE_USB_DEVICE == ENABLED)\
                                                    &&  (VSF_USBD_USE_CDC == ENABLED)\
                                                    &&  (VSF_USBD_USE_CDCNCM == ENABLED)
#   endif
#endif
#if VSF_USE_LWIP == ENABLED || defined(__WIN__)
#   define VSF_USE_TCPIP                                ENABLED
#   define VSF_USE_MBEDTLS                              ENABLED
#endif

// Host netdrv backend: use Npcap/WinPcap on Windows.
#if defined(__WIN__) && (VSF_USE_TCPIP == ENABLED)
#   define VSF_NETDRV_USE_WPCAP                         ENABLED
// wpcap netlink IRQ-thread priority; not defined by the driver itself.
#   define VSF_NETDRV_WPCAP_CFG_HW_PRIORITY             vsf_arch_prio_0
#endif

#define VSF_USE_FREERTOS                                ENABLED
#   define VSF_FREERTOS_CFG_USE_SEMPHR                  ENABLED
#   define VSF_FREERTOS_CFG_USE_EVENT_GROUPS            ENABLED
#   define VSF_FREERTOS_CFG_USE_NOTIFY                  ENABLED
#   define VSF_FREERTOS_CFG_USE_TIMERS                  ENABLED
#   define VSF_FREERTOS_CFG_USE_STREAM_BUFFER           ENABLED
#   define VSF_FREERTOS_CFG_USE_QUEUESET                ENABLED

#define VSF_USE_ESPIDF                                  ENABLED
#   define VSF_ESPIDF_CFG_USE_PARTITION                 ENABLED
#   define VSF_ESPIDF_CFG_USE_NVS                       ENABLED
#   define VSF_ESPIDF_CFG_USE_ESP_FLASH                 ENABLED
#   define VSF_ESPIDF_CFG_USE_VFS                       ENABLED
#   define VSF_ESPIDF_CFG_USE_LITTLEFS                  ENABLED
#   define VSF_ESPIDF_CFG_USE_FATFS                     ENABLED
#   if VSF_USE_LWIP == ENABLED && VSF_USE_TCPIP == ENABLED
#       define VSF_ESPIDF_CFG_USE_NETIF                 ENABLED
#   endif
#   if VSF_USE_TCPIP == ENABLED && VSF_USE_MBEDTLS == ENABLED
#       define VSF_ESPIDF_CFG_USE_HTTP_CLIENT            ENABLED
#   endif
#   if VSF_USE_USB_HOST == ENABLED
#       define VSF_ESPIDF_CFG_USE_USB_HOST              ENABLED
#   endif
#   define VSF_ESPIDF_CFG_USE_LCD                       ENABLED
#   define VSF_ESPIDF_CFG_RINGBUF_USE_FREERTOS_QUEUESET  ENABLED

#define VSF_USE_LINUX                                   ENABLED
#   ifndef VSF_LINUX_CFG_STACKSIZE
#       define VSF_LINUX_CFG_STACKSIZE                  32768
#   endif
#   define VSF_USE_POSIX                                ENABLED
#   define VSF_LINUX_USE_SIMPLE_LIBC                    ENABLED
#   define VSF_LINUX_USE_BUSYBOX                        DISABLED
#   define VSF_LINUX_CFG_LINK_FILE                      DISABLED
#   define VSF_LINUX_USE_SOCKET                         ENABLED
#       define VSF_LINUX_SOCKET_USE_UNIX                ENABLED
#       if defined(__WIN__) || defined(__LINUX__)
            // windows and linux uses host socket for linux socket support
#           define VSF_LINUX_SOCKET_USE_INET            ENABLED
#           define VSF_LINUX_SOCKET_CFG_WRAPPER         ENABLED
#           define VSF_LINUX_SOCKET_USE_NETLINK         DISABLED
#       else
#           define VSF_LINUX_SOCKET_USE_INET            VSF_USE_TCPIP
#       endif
#   define VSF_LINUX_USE_PROCFS                         DISABLED
#   define VSF_LINUX_USE_DEVFS                          DISABLED
#       define VSF_LINUX_DEVFS_USE_RAND                 DISABLED
#       if VSF_USE_AUDIO == ENABLED && !defined(VSF_LINUX_DEVFS_USE_ALSA)
#           define VSF_LINUX_DEVFS_USE_ALSA             DISABLED
#       endif
#   define VSF_LINUX_CFG_FD_BITMAP_SIZE                 256
#   if VSF_USE_USB_HOST == ENABLED && VSF_USBH_USE_LIBUSB == ENABLED
#       define VSF_LINUX_USE_LIBUSB                     DISABLED
#   else
#       define VSF_LINUX_USE_LIBUSB                     DISABLED
#   endif
#   define VSF_LINUX_CFG_PLS_NUM                        16
#   define VSF_LINUX_CFG_TLS_NUM                        64
#   if VSF_USE_LOADER == ENABLED
#       define VSF_LINUX_USE_APPLET                     DISABLED
#   endif
#   define VSF_LINUX_USE_SCRIPT                         DISABLED
#   define VSF_LINUX_CFG_INIT_SCRIPT_FILE               "/root/.profile"
#   define VSF_LINUX_CFG_PATH                           "/bin:/usr/bin"
#   ifdef __WIN__
// DO NOT enable VSF_LINUX_CFG_WRAPPER if possible.
//  Instead of enable VSF_LINUX_CFG_WRAPPER for windows if there are conflicts,
//  we recommend to add wrapper for dedicated Windows APIs in coresponding header.
//#       define VSF_LINUX_CFG_WRAPPER                    ENABLED
#   endif

#ifndef __CPU_WEBASSEMBLY__
#   define VSF_USE_SIMPLE_SPRINTF                       ENABLED
#   define VSF_USE_SIMPLE_SWPRINTF                      ENABLED
#       define VSF_SIMPLE_SPRINTF_SUPPORT_FLOAT         ENABLED
#   define VSF_USE_SIMPLE_SSCANF                        ENABLED
#endif

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ INCLUDES ======================================*/

#endif
/* EOF */
