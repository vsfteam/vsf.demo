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

/*============================ MACROS ========================================*/

// defined disabled hal before including vsf_board_cfg.h
#define VSF_HAL_USE_I2C                                 DISABLED
#define VSF_HAL_USE_MMC                                 DISABLED
#define VSF_HAL_USE_USART                               DISABLED
#define VSF_HAL_USE_RNG                                 DISABLED
#define VSF_HAL_USE_SPI                                 DISABLED

/*============================ INCLUDES ======================================*/

#include "vsf_board_cfg.h"

/*============================ MACROS ========================================*/

#define VSF_KERNEL_CFG_SUPPORT_THREAD                   DISABLED
#define VSF_KERNEL_CFG_CPU_USAGE                        DISABLED
#define VSF_KERNEL_CFG_SUPPORT_CALLBACK_TIMER           DISABLED
#define VSF_KERNEL_CFG_EDA_SUBCALL_HAS_RETURN_VALUE     DISABLED

#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_TRACE                                   ENABLED
#define VSF_USE_SCSI                                    ENABLED
#   define VSF_SCSI_USE_VIRTUAL_SCSI                    ENABLED
#   define VSF_SCSI_USE_MAL_SCSI                        ENABLED
#define VSF_USE_MAL                                     ENABLED
#   define VSF_MAL_USE_FAKEFAT32_MAL                    ENABLED
#define VSF_USE_FS                                      ENABLED
#   define VSF_FS_USE_MEMFS                             ENABLED
#   define VSF_FS_USE_FATFS                             ENABLED

#define VSF_USE_USB_DEVICE                              ENABLED
#   define VSF_USBD_USE_MSC                             ENABLED

// Application configuration
#define APP_CFG_USBD_VID                                0xA7A8
#define APP_CFG_USBD_PID                                0x2349

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif
/* EOF */
