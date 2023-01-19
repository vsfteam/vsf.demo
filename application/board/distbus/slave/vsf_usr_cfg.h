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

#undef VSF_HAL_USE_DAC
#define VSF_HAL_USE_DAC                                 DISABLED

#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_TRACE                                   ENABLED

#define VSF_USE_DISTBUS                                 ENABLED
#   define VSF_HAL_DISTBUS_CFG_MTU                      512

#define APP_DISTBUS_CFG_TRANSPORT_USBD_CDCACM
#define APP_DISTBUS_CFG_POOL_NUM                        16

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ INCLUDES ======================================*/

#ifdef APP_DISTBUS_CFG_TRANSPORT_USBD_CDCACM
#   include "transport/usbd_cdcacm/vsf_distbus_transport_cfg.h"
#else
#   error please select one transport supported
#endif

#endif
/* EOF */
