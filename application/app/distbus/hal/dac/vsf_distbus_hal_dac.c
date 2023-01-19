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

/*
 * distbus hal dac
 *
 * Dependency:
 * Board:
 */

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

#if VSF_HAL_USE_DAC == ENABLED

// define VSF_HAL_USE_DISTBUS_XXXX for constants in header
#undef VSF_HAL_USE_DISTBUS
#define VSF_HAL_USE_DISTBUS                 ENABLED
#undef VSF_HAL_DISTBUS_USE_DAC
#define VSF_HAL_DISTBUS_USE_DAC             ENABLED

#define __VSF_DISTBUS_CLASS_INHERIT__
#define __VSF_DISTBUS_HAL_DAC_CLASS_IMPLEMENT
// for hal_distbus_dac constants
#define __VSF_HAL_DISTBUS_DAC_CLASS_INHERIT__
#include "./vsf_distbus_hal_dac.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static bool __vsf_distbus_hal_dac_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static const vsf_distbus_service_info_t __vsf_distbus_hal_dac_service_info = {
    .mtu                    = VSF_HAL_DISTBUS_CFG_MTU,
    .addr_range             = VSF_HAL_DISTBUS_DAC_CMD_ADDR_RANGE,
    .handler                = __vsf_distbus_hal_dac_service_msghandler,
};

/*============================ IMPLEMENTATION ================================*/

static bool __vsf_distbus_hal_dac_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg)
{
    vsf_distbus_hal_dac_t *hal_distbus_dac = container_of(service, vsf_distbus_hal_dac_t, service);
    uint8_t *data = (uint8_t *)&msg->header + sizeof(msg->header);
    uint32_t datalen = msg->header.datalen;
    bool retain_msg = false;

    switch (msg->header.addr) {
    default:
        VSF_HAL_ASSERT(false);
        break;
    }
    return retain_msg;
}

void vsf_distbus_hal_dac_init(vsf_distbus_t *distbus, vsf_distbus_hal_dac_t *distbus_hal_dac)
{
    distbus_hal_dac->distbus = distbus;
    distbus_hal_dac->service.info = &__vsf_distbus_hal_dac_service_info;
    vsf_distbus_register_service(distbus, &distbus_hal_dac->service);
}

#endif      // VSF_HAL_USE_DAC
