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

// re-write VSF_ASSERT to bypass __EXPR string
extern void vsf_trace_assert(const char *expr, const char *file, int line, const char *func);
#define VSF_ASSERT(__EXPR)                                                      \
    ((__EXPR)                                                                   \
    ? (void)0                                                                   \
    : vsf_trace_assert(NULL, __FILE__, __LINE__, __FUNCTION__))

#include "vsf_board_cfg.h"

/*============================ MACROS ========================================*/

// kernel
#define VSF_OS_CFG_EDA_FRAME_POOL_SIZE                  64
#define VSF_OS_CFG_EDA_FRAME_POOL_EXTRA_SIZE            128

// components

#define VSF_USE_LITTLEFS                                ENABLED
#define VSF_USE_TRACE                                   ENABLED
#define VSF_USE_SIMPLE_STREAM                           ENABLED
// currently, dynamic loader is not support for wasm in vsf
#ifdef __CPU_WEBASSEMBLY__
#   define VSF_USE_LOADER                               DISABLED
#else
#   define VSF_USE_LOADER                               ENABLED
#endif
#   define VSF_LOADER_USE_ELF                           ENABLED
#define VSF_USE_SCSI                                    ENABLED
#   define VSF_SCSI_USE_VIRTUAL_SCSI                    ENABLED
#   define VSF_SCSI_USE_MAL_SCSI                        ENABLED
#define VSF_USE_MAL                                     ENABLED
#   define VSF_MAL_USE_FAKEFAT32_MAL                    ENABLED
#   define VSF_MAL_USE_HW_FLASH_MAL                     ENABLED
#define VSF_USE_FS                                      ENABLED
#   define VSF_FS_USE_ROMFS                             ENABLED
#   define VSF_FS_USE_MEMFS                             ENABLED
#   define VSF_FS_USE_FATFS                             ENABLED
#   define VSF_FS_USE_LITTLEFS                          VSF_USE_LITTLEFS
#define VSF_USE_INPUT                                   ENABLED
#if VSF_USE_LWIP == ENABLED
#   define VSF_USE_TCPIP                                ENABLED
#   define VSF_USE_MBEDTLS                              ENABLED
#endif
#ifndef VSF_USE_USB_DEVICE
#   define VSF_USE_USB_DEVICE                           ENABLED
#       define VSF_USBD_USE_MSC                         ENABLED
#endif
#ifndef VSF_USE_USB_HOST
#   define VSF_USE_USB_HOST                             ENABLED
#       define VSF_USBH_USE_HUB                         ENABLED
#       define VSF_USBH_USE_LIBUSB                      ENABLED
#       define VSF_USBH_USE_CDC                         ENABLED
#       define VSF_USBH_USE_ECM                         ENABLED
#       define VSF_USBH_USE_HID                         ENABLED
#       define VSF_USBH_USE_DS4                         ENABLED
#       define VSF_USBH_USE_DS5                         ENABLED
#       define VSF_USBH_USE_NSPRO                       ENABLED
#       define VSF_USBH_USE_XB360                       ENABLED
#       define VSF_USBH_USE_XB1                         ENABLED
#       define VSF_USBH_USE_MSC                         ENABLED
#endif

#define VSF_USE_LINUX                                   ENABLED
#   define VSF_LINUX_CFG_STACKSIZE                      8192
#   define VSF_USE_POSIX                                ENABLED
#   define VSF_LINUX_USE_SIMPLE_LIBC                    ENABLED
#   define VSF_LINUX_USE_BUSYBOX                        ENABLED
#   define VSF_LINUX_CFG_LINK_FILE                      ENABLED
#   define VSF_LINUX_USE_SOCKET                         ENABLED
#       define VSF_LINUX_SOCKET_USE_UNIX                ENABLED
#       define VSF_LINUX_SOCKET_USE_INET                VSF_USE_TCPIP
#   define VSF_LINUX_USE_PROCFS                         ENABLED
#   define VSF_LINUX_USE_DEVFS                          ENABLED
#       define VSF_LINUX_DEVFS_USE_RAND                 ENABLED
#   define VSF_LINUX_CFG_FD_BITMAP_SIZE                 256
#   if VSF_USE_USB_HOST == ENABLED && VSF_USBH_USE_LIBUSB == ENABLED
#       define VSF_LINUX_USE_LIBUSB                     ENABLED
#   else
#       define VSF_LINUX_USE_LIBUSB                     DISABLED
#   endif
#   define VSF_LINUX_CFG_PLS_NUM                        16
#   define VSF_LINUX_CFG_TLS_NUM                        64
#   if VSF_USE_LOADER == ENABLED
#       define VSF_LINUX_USE_APPLET                     ENABLED
#   endif
#   define VSF_LINUX_USE_SCRIPT                         ENABLED
#   define VSF_LINUX_CFG_INIT_SCRIPT_FILE               "/usr/etc/startup.sh"
#   define VSF_LINUX_CFG_PATH                           "/bin:/usr/bin"

#ifndef __CPU_WEBASSEMBLY__
#   define VSF_USE_SIMPLE_SPRINTF                       ENABLED
#   define VSF_USE_SIMPLE_SSCANF                        ENABLED
#endif

//#define VSF_LINUX_SIMPLE_STDLIB_CFG_HEAP_MONITOR        ENABLED
//#define VSF_LINUX_SIMPLE_STDLIB_HEAP_MONITOR_TRACE_DEPTH    1024

// APP configuration

#define APP_CFG_USBD_VID                                0xA7A8
#define APP_CFG_USBD_PID                                0x2349

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ INCLUDES ======================================*/

#endif
/* EOF */
