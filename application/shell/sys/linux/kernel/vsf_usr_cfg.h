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
    : vsf_trace_assert(NULL, __FILE__, __LINE__, __FUNCTION__))

#include "vsf_board_cfg.h"
// do not compare VSF_BOARD_USE_EXT_GAMEPAD with ENABLED here, because ENABLED maybe undefined
#if VSF_BOARD_USE_EXT_GAMEPAD
#   include "ext/gamepad/vsf_board_ext_gamepad_cfg.h"
#endif

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

#define VSF_USE_LITTLEFS                                ENABLED
#define VSF_USE_TRACE                                   ENABLED
#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_FIFO                                    ENABLED
#ifndef VSF_USE_LOADER
// currently, dynamic loader is not support for wasm in vsf
#   ifdef __CPU_WEBASSEMBLY__
#       define VSF_USE_LOADER                           DISABLED
#   else
#       define VSF_USE_LOADER                           ENABLED
#   endif
#endif
#   define VSF_LOADER_USE_ELF                           ENABLED
#define VSF_USE_SCSI                                    ENABLED
#   define VSF_SCSI_USE_VIRTUAL_SCSI                    ENABLED
#   define VSF_SCSI_USE_MAL_SCSI                        ENABLED
#define VSF_USE_MAL                                     ENABLED
#   define VSF_MAL_USE_SDMMC_MAL                        ENABLED
#   define VSF_MAL_USE_FAKEFAT32_MAL                    ENABLED
#   define VSF_MAL_USE_SCSI_MAL                         ENABLED
#   define VSF_MAL_USE_FLASH_MAL                        ENABLED
#define VSF_USE_FS                                      ENABLED
#   define VSF_FS_USE_ROMFS                             ENABLED
#   define VSF_FS_USE_MEMFS                             ENABLED
#   define VSF_FS_USE_FATFS                             ENABLED
#   define VSF_FS_USE_EXFATFS                           ENABLED
#   define VSF_FS_USE_LITTLEFS                          VSF_USE_LITTLEFS
#define VSF_USE_INPUT                                   ENABLED
#if VSF_USE_LWIP == ENABLED || defined(__WIN__)
#   define VSF_USE_TCPIP                                ENABLED
#   define VSF_USE_MBEDTLS                              ENABLED
#endif
#ifndef VSF_USE_USB_DEVICE
#   define VSF_USE_USB_DEVICE                           ENABLED
#       define VSF_USBD_USE_MSC                         ENABLED
#endif
#ifndef VSF_USE_USB_HOST
#   define VSF_USE_USB_HOST                             ENABLED
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
#       define VSF_USBH_USE_UAC                         ENABLED
#       define VSF_USBH_USE_UVC                         ENABLED
#endif

#define VSF_USE_LINUX                                   ENABLED
#   ifndef VSF_LINUX_CFG_STACKSIZE
#       define VSF_LINUX_CFG_STACKSIZE                  8192
#   endif
#   define VSF_USE_POSIX                                ENABLED
#   define VSF_LINUX_USE_SIMPLE_LIBC                    ENABLED
#   define VSF_LINUX_USE_BUSYBOX                        ENABLED
#   define VSF_LINUX_CFG_LINK_FILE                      ENABLED
#   define VSF_LINUX_USE_SOCKET                         ENABLED
#       define VSF_LINUX_SOCKET_USE_UNIX                ENABLED
#       if defined(__WIN__) || defined(__LINUX__)
            // windows and linux uses host socket for linux socket support
#           define VSF_LINUX_SOCKET_USE_INET            ENABLED
#           define VSF_LINUX_SOCKET_CFG_WRAPPER         ENABLED
#       else
#           define VSF_LINUX_SOCKET_USE_INET            VSF_USE_TCPIP
#       endif
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

#define VSF_LINUX_SIMPLE_STDLIB_CFG_HEAP_MONITOR        ENABLED

// APP configuration

#define APP_CFG_USBD_VID                                0xA7A8
#define APP_CFG_USBD_PID                                0x2349

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

// application APIs

extern void app_mdns_rename(const char *hostname);
extern int app_mdns_update_txt(void *service, const char **txt, unsigned char txt_num);
extern void * app_mdns_update_service(void *record_orig, const char *name,
        const char *service, unsigned short port, int is_tcp,
        const char **txt, unsigned char txt_num);
extern void app_mdns_remove_service(void *service);

extern int app_config_read(const char *cfgname, char *cfgvalue, int valuelen);
extern int app_config_write(const char *cfgname, char *cfgvalue);

/*============================ INCLUDES ======================================*/

#endif
/* EOF */
