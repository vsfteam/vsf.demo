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

#include "vsf_board_cfg.h"

/*============================ MACROS ========================================*/

#define VSF_ASSERT(...)                                 if (!(__VA_ARGS__)) {while(1);};

#define VSF_USE_SIMPLE_SPRINTF                          ENABLED
#   define VSF_SIMPLE_SPRINTF_SUPPORT_FLOAT             ENABLED
#   define VSF_SIMPLE_SPRINTF_SUPPORT_IPMAC             ENABLED

#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_TRACE                                   ENABLED
#define VSF_USE_MAL                                     ENABLED
#define VSF_USE_FS                                      ENABLED
#define VSF_USE_INPUT                                   ENABLED
#define VSF_USE_UI                                      ENABLED
#define VSF_USE_LVGL                                    ENABLED
#   define VSF_LVGL_IMP_WAIT_CB                         ENABLED

#define VSF_USE_LINUX                                   ENABLED
#   define VSF_LINUX_CFG_STACKSIZE                      (32 * 1024)
#   define VSF_USE_POSIX                                ENABLED
#   define VSF_LINUX_USE_BUSYBOX                        ENABLED
#   define VSF_LINUX_CFG_FD_BITMAP_SIZE                 32
#   define VSF_LINUX_USE_SIMPLE_LIBC                    ENABLED

#   define APP_USE_LINUX_MOUNT_DEMO                     ENABLED
#   define VSF_LINUX_CFG_INIT_SCRIPTS                                           \
            "mkdir -p /mnt/hostfs",                                             \
            "mount -t winfs . /mnt/hostfs",                                     \
            "cd /mnt/hostfs",

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif
/* EOF */
