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

#ifndef __VSF_DISTBUS_TRANSPORT_USBD_CDCACM_CFG_H__
#define __VSF_DISTBUS_TRANSPORT_USBD_CDCACM_CFG_H__

/*============================ INCLUDES ======================================*/

#define VSF_DISTBUS_TRANSPORT_USE_STREAM                ENABLED
#include "../stream/vsf_distbus_transport_stream_cfg.h"

/*============================ MACROS ========================================*/

// for block stream
#define VSF_USE_FIFO                                    ENABLED

#define VSF_USE_USB_DEVICE                              ENABLED
#   define VSF_USBD_USE_CDC                             ENABLED
#       define VSF_USBD_USE_CDCACM                      ENABLED

// Application configuration
#define APP_CFG_USBD_VID                                0xA7A8
#define APP_CFG_USBD_PID                                0x2348

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif
/* EOF */
