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
 * vsf_board distbus slave for hal
 *
 * Dependency:
 * Board:
 *   VSF_USB_DC0
 *
 * Source:
 *   application/app/distbus/*
 *
 * Include Directories:
 *   application/app/distbus/transport/specific transport/*
 *   application/app/distbus/hal/*
 *
 * Extra:
 *   dependency for the specific transport
 */

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "vsf_board.h"

#include "hal/vsf_distbus_hal.h"
#ifdef APP_DISTBUS_CFG_TRANSPORT_USBD_CDCACM
#   include "transport/usbd_cdcacm/vsf_distbus_transport.h"
#else
#   error please select one transport supported
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct __user_distbus_msg_t {
    implement(vsf_distbus_msg_t)
    uint8_t buffer[VSF_HAL_DISTBUS_CFG_MTU];
} __user_distbus_msg_t;

dcl_vsf_pool(__user_distbus_msg_pool)
def_vsf_pool(__user_distbus_msg_pool, __user_distbus_msg_t)

typedef struct __user_distbus_t {
    vsf_distbus_t                           distbus;
    vsf_pool(__user_distbus_msg_pool)       msg_pool;
} __user_distbus_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

static void __user_distbus_on_error(vsf_distbus_t *distbus);
static void * __user_distbus_alloc_msg(uint_fast32_t size);
static void __user_distbus_free_msg(void *msg);

/*============================ LOCAL VARIABLES ===============================*/

static __user_distbus_t __user_distbus = {
    .distbus                    = {
        .op                     = {
            .mem                = {
                .alloc_msg      = __user_distbus_alloc_msg,
                .free_msg       = __user_distbus_free_msg,
            },
            .bus                = {
                .init           = vsf_distbus_transport_init,
                .send           = vsf_distbus_transport_send,
                .recv           = vsf_distbus_transport_recv,
            },
            .on_error           = __user_distbus_on_error,
        },
    },
};

static vsf_distbus_hal_t __vsf_distbus_hal = {
    0
};

/*============================ IMPLEMENTATION ================================*/

imp_vsf_pool(__user_distbus_msg_pool, __user_distbus_msg_t)

static void __user_distbus_on_error(vsf_distbus_t *distbus)
{
    VSF_ASSERT(false);
}

static void * __user_distbus_alloc_msg(uint_fast32_t size)
{
    VSF_ASSERT(size <= VSF_HAL_DISTBUS_CFG_MTU);
    return VSF_POOL_ALLOC(__user_distbus_msg_pool, &__user_distbus.msg_pool);
}

static void __user_distbus_free_msg(void *msg)
{
    VSF_POOL_FREE(__user_distbus_msg_pool, &__user_distbus.msg_pool, msg);
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
#if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#endif

    VSF_POOL_INIT(__user_distbus_msg_pool, &__user_distbus.msg_pool, APP_DISTBUS_CFG_POOL_NUM);
    vsf_distbus_init(&__user_distbus.distbus);

    // initialize services
    vsf_distbus_hal_init(&__user_distbus.distbus, &__vsf_distbus_hal);

    vsf_distbus_start(&__user_distbus.distbus);
    return 0;
}
