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

#if VSF_USE_DISTBUS == ENABLED

// define VSF_HAL_USE_DISTBUS_XXXX for constants in header
#undef VSF_HAL_USE_DISTBUS
#define VSF_HAL_USE_DISTBUS                 ENABLED

#define __VSF_DISTBUS_CLASS_INHERIT__
// for hal_distbus constants
#define __VSF_HAL_DISTBUS_CLASS_INHERIT__
#define __VSF_DISTBUS_HAL_CLASS_IMPLEMENT
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
    vsf_distbus_hal_t *distbus_hal = vsf_container_of(service, vsf_distbus_hal_t, service);
    uint32_t datalen = msg->header.datalen;
    uint8_t *data;

    union {
        void *ptr;
    } u_arg;
    u_arg.ptr = (uint8_t *)&msg->header + sizeof(msg->header);

    switch (msg->header.addr) {
    case VSF_HAL_DISTBUS_CMD_CONNECT:
        msg = vsf_distbus_alloc_msg(distbus_hal->distbus, VSF_HAL_DISTBUS_CFG_MTU, &data);
        if (NULL == msg) {
            VSF_ASSERT(false);
            break;
        }

        datalen = VSF_HAL_DISTBUS_CFG_MTU;
        uint32_t reallen, i;

#define VSF_DISTBUS_HAL_PREPARE_DECLARE_CMD(__TYPE)                             \
        if (distbus_hal->__TYPE.dev_num > 0) {                                  \
            VSF_ASSERT(datalen >= 2);                                           \
            *data++ = VSF_MCONNECT(VSF_HAL_DISTBUS_, __TYPE);                   \
            *data++ = distbus_hal->__TYPE.dev_num;                              \
            datalen -= 2;                                                       \
            for (i = 0; i < distbus_hal->__TYPE.dev_num; i++) {                 \
                reallen = VSF_MCONNECT(vsf_distbus_hal_, __TYPE, _declare)(     \
                            &distbus_hal->__TYPE.dev[i], data, datalen);        \
                if (reallen > datalen) {                                        \
                    VSF_ASSERT(false);                                          \
                    break;                                                      \
                }                                                               \
                VSF_MCONNECT(vsf_distbus_hal_, __TYPE, _register)(              \
                            distbus_hal->distbus, &distbus_hal->__TYPE.dev[i]); \
                data += reallen;                                                \
                datalen -= reallen;                                             \
            }                                                                   \
        }
#define __VSF_DISTBUS_HAL_ENUM  VSF_DISTBUS_HAL_PREPARE_DECLARE_CMD
#include "vsf_distbus_hal_enum.inc"

        msg->header.datalen = VSF_HAL_DISTBUS_CFG_MTU - datalen;
        msg->header.addr = VSF_HAL_DISTBUS_CMD_DECLARE;
        vsf_distbus_send_msg(distbus_hal->distbus, &distbus_hal->service, msg);

        if (!distbus_hal->remote_connected) {
            distbus_hal->remote_connected = true;
            if (distbus_hal->on_remote_connected != NULL) {
                distbus_hal->on_remote_connected(distbus_hal);
            }
        }
        break;
    case VSF_HAL_DISTBUS_CMD_DECLARE:
        if (!distbus_hal->remote_declared) {
            distbus_hal->remote_declared = true;
        }
        break;
    }
    return false;
}

void vsf_distbus_hal_register(vsf_distbus_t *distbus, vsf_distbus_hal_t *distbus_hal)
{
    distbus_hal->remote_declared = false;
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

#endif      // VSF_USE_DISTBUS
