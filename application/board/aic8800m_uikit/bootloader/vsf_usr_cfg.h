/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
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
/*============================ MACROS ========================================*/

// avoid to use assert to save a lot of resources
#define VSF_ASSERT(...)                                 if (!(__VA_ARGS__)) {while(1);}

#define VSF_HAL_USE_DEBUG_STREAM                        DISABLED
#define VSF_HAL_USE_I2S                                 DISABLED
#define VSF_HAL_USE_SDIO                                DISABLED
#define VSF_HAL_USE_RNG                                 DISABLED
#define VSF_HAL_USE_SPI                                 DISABLED
#define VSF_HAL_USE_I2C                                 DISABLED
#define VSF_HAL_USE_ADC                                 DISABLED
#define VSF_HAL_USE_USART                               DISABLED

#define VSF_OS_CFG_MAIN_MODE                            VSF_OS_CFG_MAIN_MODE_IDLE
//#define VSF_OS_CFG_PRIORITY_NUM                         1
#define VSF_OS_CFG_EDA_FRAME_POOL_SIZE                  8
#define VSF_OS_CFG_EDA_FRAME_POOL_EXTRA_SIZE            128
#define VSF_KERNEL_CFG_SUPPORT_THREAD                   ENABLED
#define VSF_KERNEL_CFG_SUPPORT_SYNC                     ENABLED
#define VSF_KERNEL_CFG_SUPPORT_EDA_QUEUE                ENABLED
#define VSF_KERNEL_CFG_ALLOW_KERNEL_BEING_PREEMPTED     ENABLED
#define VSF_KERNEL_CFG_EDA_SUPPORT_ON_TERMINATE         ENABLED
#define VSF_KERNEL_CFG_SUPPORT_CALLBACK_TIMER           ENABLED
#define VSF_KERNEL_CFG_EDA_SUPPORT_TIMER                ENABLED

#define VSF_KERNEL_CFG_SUPPORT_BITMAP_EVENT             DISABLED
#define VSF_USE_KERNEL_SIMPLE_SHELL                     DISABLED
#define VSF_KERNEL_CFG_EDA_SUBCALL_HAS_RETURN_VALUE     DISABLED
#define VSF_KERNEL_CFG_EDA_SUPPORT_PT                   DISABLED

#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_TRACE                                   DISABLED

#define VSF_USE_MAL                                     ENABLED
#   define VSF_MAL_USE_FAKEFAT32_MAL                    ENABLED
#   define VSF_MAL_USE_FLASH_MAL                        ENABLED
#define VSF_USE_FS                                      ENABLED
#   define VSF_FS_CFG_FILE_POOL_SIZE                    1
#   define VSF_FS_CFG_FILE_POOL_FILE_SIZE               64
#   define VSF_FS_USE_MEMFS                             ENABLED
#   define VSF_FS_USE_FATFS                             ENABLED
#define VSF_USE_SCSI                                    ENABLED
#   define VSF_SCSI_USE_MAL_SCSI                        ENABLED
#define VSF_USE_USB_DEVICE                              ENABLED
#   define VSF_USBD_USE_MSC                             ENABLED
#define VSF_USE_POOL                                    ENABLED
//#define VSF_USE_HEAP                                    DISABLED

// application configuration
#define APP_CFG_USBD_VID                                0xA7A8
#define APP_CFG_USBD_PID                                0x2348

#define __MSCBOOT_BOOTLOADER_SIZE                       (64 * 1024)
// reserve __MSCBOOT_BOOTLOADER_SIZE and 16K reserved configuration area for wifi
#define MSCBOOT_CFG_FW_SIZE                             ((2 * 1024 * 1024) - __MSCBOOT_BOOTLOADER_SIZE - (16 * 1024))
#define MSCBOOT_CFG_FW_ADDR                             __MSCBOOT_BOOTLOADER_SIZE

/*============================ INCLUDES ======================================*/

#include "vsf_board_cfg.h"

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif
/* EOF */
