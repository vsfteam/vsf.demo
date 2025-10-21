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

#ifndef __VSF_DISTBUS_TRANSPORT_H__
#define __VSF_DISTBUS_TRANSPORT_H__

/*============================ INCLUDES ======================================*/

#include "vsf_cfg.h"

#if     VSF_DISTBUS_TRANSPORT_USE_USBD_CDCACM == ENABLED
#   include "./usbd_cdcacm/vsf_distbus_transport_usbd_cdcacm.h"
typedef vsf_distbus_transport_usbd_cdcacm_t     vsf_distbus_transport_t;
#   define vsf_distbus_transport_init           vsf_distbus_transport_usbd_cdcacm_init
#   define vsf_distbus_transport_send           vsf_distbus_transport_usbd_cdcacm_send
#   define vsf_distbus_transport_recv           vsf_distbus_transport_usbd_cdcacm_recv
#   define vsf_distbus_transport_is_sending     vsf_distbus_transport_usbd_cdcacm_is_sending
#elif   VSF_DISTBUS_TRANSPORT_USE_STREAM == ENABLED
#   include "./stream/vsf_distbus_transport_stream.h"
typedef vsf_distbus_transport_stream_t          vsf_distbus_transport_t;
#   define vsf_distbus_transport_init           vsf_distbus_transport_stream_init
#   define vsf_distbus_transport_send           vsf_distbus_transport_stream_send
#   define vsf_distbus_transport_recv           vsf_distbus_transport_stream_recv
#   define vsf_distbus_transport_is_sending     vsf_distbus_transport_stream_is_sending
#else
#   error please select transport
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_DISTBUS_TRANSPORT_H__
