/*****************************************************************************
 *   Copyright(C)2009-2026 by VSF Team                                       *
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

#ifndef __VSF_USR_CFG_LINUX_H__
#define __VSF_USR_CFG_LINUX_H__

#define VSF_USE_KERNEL                          ENABLED
#define VSF_KERNEL_CFG_SUPPORT_THREAD           ENABLED
#define VSF_OS_CFG_MAIN_MODE                    VSF_OS_CFG_MAIN_MODE_THREAD

#define VSF_USE_FS                              ENABLED
#define VSF_USE_MAL                             ENABLED
#define VSF_USE_LINUX                           ENABLED
#define VSF_USE_POSIX                           ENABLED
#define VSF_LINUX_USE_SIMPLE_LIBC               ENABLED
#define VSF_LINUX_CFG_STACKSIZE                 4096
#define VSF_LINUX_CFG_FD_BITMAP_SIZE            32
#define VSF_LINUX_USE_DEVFS                     ENABLED

#endif
