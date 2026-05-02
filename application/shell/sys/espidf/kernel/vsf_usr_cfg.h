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

#ifndef VSF_USE_LWIP
#   define VSF_USE_LWIP                                 (VSF_USE_USB_DEVICE == ENABLED)\
                                                    &&  (VSF_USBD_USE_CDC == ENABLED)\
                                                    &&  (VSF_USBD_USE_CDCNCM == ENABLED)
#endif
#if VSF_USE_LWIP == ENABLED || defined(__WIN__)
#   define VSF_USE_TCPIP                                ENABLED
#   define VSF_USE_MBEDTLS                              ENABLED
#endif

#define VSF_USE_ESPIDF                                  ENABLED

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ INCLUDES ======================================*/

#endif
/* EOF */
