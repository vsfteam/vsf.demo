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

#ifndef __VSF_DISTBUS_TRANSPORT_USBD_CDCACM_H__
#define __VSF_DISTBUS_TRANSPORT_USBD_CDCACM_H__

#include "vsf_cfg.h"

#if VSF_DISTBUS_TRANSPORT_USE_USBD_CDCACM == ENABLED

// for bool
#include "utilities/vsf_utilities.h"
#include "../stream/vsf_distbus_transport_stream.h"

#if     defined(__VSF_DISTBUS_TRANSPORT_USBD_CDCACM_CLASS_IMPLEMENT)
#   undef __VSF_DISTBUS_TRANSPORT_USBD_CDCACM_CLASS_IMPLEMENT
#   define __VSF_CLASS_IMPLEMENT__
#elif   defined(__VSF_DISTBUS_TRANSPORT_USBD_CDCACM_CLASS_INHERIT__)
#   undef __VSF_DISTBUS_TRANSPORT_USBD_CDCACM_CLASS_INHERIT__
#   define __VSF_CLASS_INHERIT__
#endif

#include "utilities/ooc_class.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ INCLUDES ======================================*/
/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

vsf_class(vsf_distbus_transport_usbd_cdcacm_t) {
    public_member(
        implement(vsf_distbus_transport_stream_t)
    )
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern bool vsf_distbus_transport_usbd_cdcacm_init(void *transport, void *p, void (*on_inited)(void *p));
extern bool vsf_distbus_transport_usbd_cdcacm_send(void *transport, uint8_t *buffer, uint_fast32_t size, void *p, void (*on_sent)(void *p));
extern bool vsf_distbus_transport_usbd_cdcacm_recv(void *transport, uint8_t *buffer, uint_fast32_t size, void *p, void (*on_recv)(void *p));
extern bool vsf_distbus_transport_usbd_cdcacm_is_sending(void *transport);

#ifdef __cplusplus
}
#endif

#endif      // VSF_DISTBUS_TRANSPORT_USE_USBD_CDCACM
#endif      // __VSF_DISTBUS_TRANSPORT_USBD_CDCACM_H__
