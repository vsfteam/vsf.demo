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
 * distbus hal pwm
 *
 * Dependency:
 * Board:
 */

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

#if VSF_HAL_USE_PWM == ENABLED

#define __VSF_DISTBUS_CLASS_INHERIT__
#define __VSF_DISTBUS_HAL_PWM_CLASS_IMPLEMENT
// for hal_distbus_pwm constants
#define __VSF_HAL_DISTBUS_PWM_CLASS_INHERIT__
#include "./vsf_distbus_hal_pwm.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static bool __vsf_distbus_hal_pwm_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static const vsf_distbus_service_info_t __vsf_distbus_hal_pwm_service_info = {
    .mtu                    = VSF_HAL_DISTBUS_CFG_MTU,
    .addr_range             = VSF_HAL_DISTBUS_PWM_CMD_ADDR_RANGE,
    .handler                = __vsf_distbus_hal_pwm_service_msghandler,
};

/*============================ IMPLEMENTATION ================================*/

static bool __vsf_distbus_hal_pwm_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg)
{
    vsf_distbus_hal_pwm_t *hal_distbus_pwm = container_of(service, vsf_distbus_hal_pwm_t, service);
    uint32_t datalen = msg->header.datalen;
    bool retain_msg = false;

    union {
        void *ptr;
    } u_arg;
    u_arg.ptr = (uint8_t *)&msg->header + sizeof(msg->header);

    switch (msg->header.addr) {
    default:
        VSF_HAL_ASSERT(false);
        break;
    }
    return retain_msg;
}

uint32_t vsf_distbus_hal_pwm_declare(vsf_distbus_hal_pwm_t *distbus_hal_pwm, uint8_t *ptr, uint32_t size)
{
    return 0;
}

void vsf_distbus_hal_pwm_register(vsf_distbus_t *distbus, vsf_distbus_hal_pwm_t *distbus_hal_pwm)
{
    distbus_hal_pwm->distbus = distbus;
    distbus_hal_pwm->service.info = &__vsf_distbus_hal_pwm_service_info;
    vsf_distbus_register_service(distbus, &distbus_hal_pwm->service);
}

#endif      // VSF_HAL_USE_PWM
