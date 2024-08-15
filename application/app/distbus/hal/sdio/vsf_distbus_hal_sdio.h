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

#ifndef __VSF_DISTBUS_HAL_SDIO_H__
#define __VSF_DISTBUS_HAL_SDIO_H__

#include "hal/vsf_hal.h"
#include "service/vsf_service.h"

#if VSF_HAL_USE_SDIO == ENABLED && VSF_USE_DISTBUS == ENABLED

#if     defined(__VSF_DISTBUS_HAL_SDIO_CLASS_IMPLEMENT)
#   define __VSF_CLASS_IMPLEMENT__
#elif   defined(__VSF_DISTBUS_HAL_SDIO_CLASS_INHERIT__)
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

vsf_class(vsf_distbus_hal_sdio_t) {
    public_member(
        vsf_sdio_t              *target;
    )
    private_member(
        vsf_distbus_service_t   service;
        vsf_distbus_t           *distbus;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern uint32_t vsf_distbus_hal_sdio_declare(vsf_distbus_hal_sdio_t *sdio, uint8_t *ptr, uint32_t size);
extern void vsf_distbus_hal_sdio_poll(vsf_distbus_hal_sdio_t *sdio);

extern void vsf_distbus_hal_sdio_register(vsf_distbus_t *distbus, vsf_distbus_hal_sdio_t *sdio);

#ifdef __cplusplus
}
#endif

#endif      // VSF_HAL_USE_SDIO && VSF_USE_DISTBUS
#endif      // __VSF_DISTBUS_HAL_SDIO_H__
