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

#ifndef __VSF_DISTBUS_HAL_DAC_H__
#define __VSF_DISTBUS_HAL_DAC_H__

#include "hal/vsf_hal.h"
#include "service/vsf_service.h"

#if VSF_HAL_USE_DAC == ENABLED

#if     defined(__VSF_DISTBUS_HAL_DAC_CLASS_IMPLEMENT)
#   define __VSF_CLASS_IMPLEMENT__
#elif   defined(__VSF_DISTBUS_HAL_DAC_CLASS_INHERIT__)
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

vsf_class(vsf_distbus_hal_dac_t) {
//    public_member(
//    )
    private_member(
        vsf_distbus_service_t   service;
        vsf_distbus_t           *distbus;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_distbus_hal_dac_init(vsf_distbus_t *distbus, vsf_distbus_hal_dac_t *distbus_hal_dac);

#ifdef __cplusplus
}
#endif

#endif      // VSF_HAL_USE_DAC
#endif      // __VSF_DISTBUS_HAL_DAC_H__
