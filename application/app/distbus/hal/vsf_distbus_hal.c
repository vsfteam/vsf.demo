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
 * distbus hal
 *
 * Dependency:
 * Board:
 */

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

// define VSF_HAL_USE_DISTBUS_XXXX for constants in header
#undef VSF_HAL_USE_DISTBUS
#define VSF_HAL_USE_DISTBUS                 ENABLED

#define __VSF_DISTBUS_CLASS_INHERIT__
// for hal_distbus constants
#define __VSF_HAL_DISTBUS_CLASS_INHERIT__
#include "./vsf_distbus_hal.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static bool __vsf_distbus_hal_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static const vsf_distbus_service_info_t __vsf_distbus_hal_service_info = {
    .mtu                    = VSF_HAL_DISTBUS_CFG_MTU,
    .addr_range             = VSF_HAL_DISTBUS_CMD_ADDR_RANGE,
    .handler                = __vsf_distbus_hal_service_msghandler,
};

/*============================ IMPLEMENTATION ================================*/

static bool __vsf_distbus_hal_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg)
{
    vsf_distbus_hal_t *distbus_hal = container_of(service, vsf_distbus_hal_t, service);
    uint8_t *data = (uint8_t *)&msg->header + sizeof(msg->header);
    uint32_t datalen = msg->header.datalen;

    switch (msg->header.addr) {
    case VSF_HAL_DISTBUS_CMD_CONNECT:
        break;
    }
    return false;
}

void vsf_distbus_hal_register(vsf_distbus_t *distbus, vsf_distbus_hal_t *distbus_hal)
{
    distbus_hal->distbus = distbus;
    distbus_hal->service.info = &__vsf_distbus_hal_service_info;
    vsf_distbus_register_service(distbus, &distbus_hal->service);
}

void vsf_distbus_hal_start(vsf_distbus_hal_t *distbus_hal)
{
    vsf_distbus_msg_t *msg = vsf_distbus_alloc_msg(distbus_hal->distbus, 0, NULL);
    VSF_HAL_ASSERT(msg != NULL);

    msg->header.addr = VSF_HAL_DISTBUS_CMD_CONNECT;
    vsf_distbus_send_msg(distbus_hal->distbus, &distbus_hal->service, msg);
}
