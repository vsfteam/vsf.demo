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

#include "vsf_board_cfg.h"

/*============================ MACROS ========================================*/

// components
#define VSF_USE_TRACE                                   ENABLED
#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_LOADER                                  ENABLED
#   define VSF_LOADER_USE_ELF                           ENABLED
#define VSF_USE_MAL                                     ENABLED
#define VSF_USE_FS                                      ENABLED
#   define VSF_FS_USE_ROMFS                             ENABLED
#define VSF_USE_INPUT                                   ENABLED
#if VSF_USE_LWIP == ENABLED
#   define VSF_USE_TCPIP                                ENABLED
#endif

#define VSF_USE_LINUX                                   ENABLED
#   define VSF_LINUX_CFG_STACKSIZE                      8192
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
#   define VSF_LINUX_CFG_FD_BITMAP_SIZE                 32
#   define VSF_LINUX_USE_LIBUSB                         VSF_USE_USB_HOST
#   define VSF_LINUX_CFG_PLS_NUM                        16
#   define VSF_LINUX_CFG_TLS_NUM                        64
#   if VSF_USE_LOADER == ENABLED
#       define VSF_LINUX_USE_APPLET                     ENABLED
#   endif
#   define VSF_LINUX_USE_SCRIPT                         ENABLED

#if defined(__WIN__)
#   define VSF_LINUX_HOSTFS_INIT_SCRIPTS                                        \
            "mkdir -p /mnt/hostfs",                                             \
            "mount -t winfs . /mnt/hostfs",                                     \
            "cd /mnt/hostfs",
#elif    defined(__LINUX__)
#   define VSF_LINUX_HOSTFS_INIT_SCRIPTS                                        \
            "mkdir -p /mnt/hostfs",                                             \
            "mount -t linfs . /mnt/hostfs",                                     \
            "cd /mnt/hostfs",
#else
#   define VSF_LINUX_HOSTFS_INIT_SCRIPTS
#endif

#define VSF_LINUX_CFG_INIT_SCRIPTS                                              \
            "echo \"vsf build on " __DATE__ "\"",                               \
            VSF_LINUX_HOSTFS_INIT_SCRIPTS                                       \
            "/bin/elfpatch",

#define VSF_USE_SIMPLE_SPRINTF                          ENABLED
#define VSF_USE_SIMPLE_SWPRINTF                         ENABLED
#define VSF_USE_SIMPLE_SSCANF                           ENABLED

// APP configuration

#define APP_USE_LINUX_MOUNT_DEMO                        ENABLED

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ INCLUDES ======================================*/

#endif
/* EOF */
